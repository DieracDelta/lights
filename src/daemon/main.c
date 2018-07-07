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

#define DEBUG 1




const char SERVERSOCKETPATH[] ="/tmp/alienfxsocket";
const char PIDPATH[] = "/var/run/alienfx";
const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid";
const char STATEFILEPATH[] = "~/.alienfx.yaml";

static int pid_fd;
static int on;

// TODO move methods to header and actually make a header file like a good boyo
void plog(const char * fmt);
void plog(const char * fmt){
  // this SHOULD have worked ...
  /* static int counter = 0; */
  /* const char initial_str[] = "ALIENFX: "; */
  /* const char end_str[] = "\n"; */



  /* int c_len = snprintf( NULL, 0, "%d", counter++); */
  /* // + 1 for null terminator */
  /* char * c_str = (char*)malloc(++c_len); */
  /* snprintf(c_str, c_len + 1, "%d", counter); */

  /* int new_fmt_size = strlen(initial_str) + strlen(c_str) + strlen(fmt) + strlen(end_str); */
  /* char * new_fmt = (char*) malloc(new_fmt_size + 1); */
  /* char * cur_ptr = new_fmt; */

  /* // TODO clean this up */
  /* strncpy(cur_ptr, initial_str, strlen(initial_str)); */
  /* cur_ptr += strlen(initial_str); */
  /* strncpy(cur_ptr, c_str, strlen(c_str)); */
  /* cur_ptr += strlen(c_str); */
  /* strncpy(cur_ptr, fmt, strlen(fmt)); */
  /* cur_ptr += strlen(fmt); */
  /* strncpy(cur_ptr, end_str, strlen(end_str)); */
  /* cur_ptr += strlen(end_str) + 1; */


  /* va_list fmt_args; */
  /* va_start(fmt_args, fmt); */
  /* // TODO add more in-depth logging */
  /* puts("what the shitter1"); */
  /* vsyslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), fmt, fmt_args); */
  /* va_end(fmt_args); */
  /* puts("what the shitter2"); */

  /* free(new_fmt); */
  /* free(c_str); */

  /* va_list fmt_args; */
  /* va_start(fmt_args, fmt); */
  /* int bruh = LOG_MAKEPRI(LOG_DAEMON, LOG_INFO); */
  /* vsyslog(bruh, fmt, fmt_args); */
  /* va_end(fmt_args); */
  // this SHOULD have worked ...

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

void main(){

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

  on = 1;

  while(on){
    // local namespace,
    plog("HI BITCH");
    int socket_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    struct sockaddr_un ;
    if(socket_fd < 0) {
      exit(EXIT_FAILURE);
      plog("failed to create socket");
    }



    sleep(100);
  }
}
