#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <sys/un.h>
#include "protocol.h"
#include "alienfx.h"

#define DEBUG 1
/* #define PACKET_SIZE 1024 */
#define NUM_OPS 5



const char SERVERSOCKETPATH[] ="/tmp/alienfxsocket.sock";
const char PIDPATH[] = "/var/run/alienfx";
const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid";
const char STATEPATH[] = "/var/lib/alienfx/";
/* const char STATEFILEPATH[] = "~/.alienfx.json"; */

static int pid_fd;
static int on;



// takes args and fills out response/performs actions
int (*response_handlers[11])(uint8_t, uint8_t*, struct alienfx_response *) = {
  get_handler,
  set_colors_handler,
  increment_colors_handler,
  decrement_colors_handler,
  toggle_pause_handler,
  NULL,
  NULL,
  NULL,
  set_profile_handler,
  increment_colors_handler,
  decrement_colors_handler,
};

// TODO move methods to header and actually make a header file like a good boyo
void plog(const char * fmt){
  static int counter = 0;
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX %d: %s \n", counter++, fmt);
  #if DEBUG
  printf("ALIENFX %d: %s \n", counter -1, fmt);
  #endif
}

void sig_handler(const int signal){
  // TODO add more extensive support for signals
  on = 0;

  // unlock and close pid
  if(lockf(pid_fd, F_ULOCK, 0) < 0) goto end;
  if(close(pid_fd) < 0) goto end;
  if(remove(PIDFILEPATH) < 0) goto end;
  goto success;

 end:
  plog("failed to close pid file");
  return;
 success:
  plog("successfully closed pid file");
  plog("EXITING");
}

// inspired by a bazillion sources, all of which just said "fork twice my frand"
// GET DAT BAGGROUND

int main(){
  fflush(stdout);

  // BOOKKEEPING
  pid_t pid = fork();

  // exit parent
  if(pid < 0) exit(EXIT_FAILURE);
  // TODO exit or _exit?
  if(pid > 0) exit(EXIT_SUCCESS);

  // be your own session
  if(setsid() < 0) exit(EXIT_FAILURE);

  int second_pid = fork();

  // exit child, leaving grandchild
  if(second_pid < 0) exit(EXIT_FAILURE);
  if(second_pid > 0) exit(EXIT_SUCCESS);


  // switch cwd to root to allow for unmounting 
  if(chdir("/") < 0) exit(EXIT_FAILURE);

  // child has full access to files
  umask(0);

  // alternative to remove stdin stdout stderr fds
  /* close(STDIN_FILENO); */
  /* close(STDOUT_FILENO); */
  /* close(STDERR_FILENO); */

  // close fds
  #if DEBUG
  for(int fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--){
    close(fd);
  }
  #endif

  // write pid to PIDFILEPATH
  // initially st had a zero in it but that's equivalent to null soo
  struct stat st = {};
  if(stat(PIDPATH, &st) < 0) {
    mkdir(PIDPATH, 0755);
  }

  pid_fd = open(PIDFILEPATH, O_RDWR|O_CREAT, 0755);
  if(pid_fd < 0) exit(EXIT_FAILURE);

  // F_TLOCK because we don't want blocking, we want error if the file is already locked
  if(lockf(pid_fd, F_TLOCK, 0) < 0) exit(EXIT_FAILURE);
  char str[69];
  sprintf(str, "%d", getpid());
  if (write(pid_fd, str, strlen(str)) < 0) exit(EXIT_FAILURE);

  // open up logs; we're not taking input
  // TODO we're not taking input right
  stdin = fopen("/dev/null", "r");
  stdout = fopen("/dev/null", "r");
  stderr = fopen("/dev/null", "r");

  // catch all the signals
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGQUIT, sig_handler);

  plog("ALIENFX has started successfully");

  // put together socket

  // TODO better logging
  remove(SERVERSOCKETPATH);

  plog("point 6");

  // put together server socket
  struct sockaddr_un socket_addr;
  memset(&socket_addr, 0, sizeof(socket_addr));
  socket_addr.sun_family = AF_LOCAL;
  strncpy(socket_addr.sun_path, SERVERSOCKETPATH, sizeof(socket_addr.sun_path));
  socket_addr.sun_path[sizeof(socket_addr.sun_path) - 1] = '\0';
  socklen_t len_socket_addr = SUN_LEN(&socket_addr);

  int socket_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
  if(socket_fd < 0) {
    exit(EXIT_FAILURE);
    // TODO log it
    printf("failed to create socket\n");
  }
  plog("point 5");

  unlink(SERVERSOCKETPATH);
  int success = bind(socket_fd, (struct sockaddr *)&socket_addr, len_socket_addr);
  if(success < 0){
    // TODO log it
    printf("failed to bind to socket with code %d", success);
    exit(EXIT_FAILURE);
  }

  // define garbage pointer in case you need to know where stuff came from
  struct sockaddr_un from_addr;
  socklen_t from_len = SUN_LEN(&from_addr);
  plog("point 4");

  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  on = 1;
  plog("point 1");
  init_profiles();
  plog("point 2");
  write_entire_profile(NULL);
  plog("point 3");

  while(on){
    plog("waiting to recieve something");
    if(recvfrom(socket_fd, packet, sizeof(struct alienfx_msg), 0, (struct sockaddr *)&from_addr, &from_len) != sizeof(struct alienfx_msg)){
      // TODO log it
      printf("failed to receive message or message did not match protocol length");
      plog("failure man");
    }
    plog("recieved something");
    // TODO else send error back
    if(packet->OP < NUM_OPS){
      plog("im doing stuff");
      int success = response_handlers[packet->OP](packet->profile_index, packet->args, resp);
      if(success < 0){
        plog("failure at responding to something");
      }
    } else{
      printf("Packet op did not match expected format");
    }
    plog("bish");
    memset(packet, 0, sizeof(struct alienfx_msg));
    memset(resp, 0, sizeof(struct alienfx_response));
    plog("finished@!");
    break;
  }

  shutdown(socket_fd, 0);
  remove(SERVERSOCKETPATH);
  free(packet);
  free(resp);
}
