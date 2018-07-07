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

#define DEBUG 1

/* const char SERVERSOCKETPATH[] = "/tmp/alienfxserver.sock"; */
/* const char CLIENTSOCKETPATH[] ="/tmp/alienfxclient.sock"; */
const char SERVERSOCKETPATH[] = "alienfxserver.sock";
const char CLIENTSOCKETPATH[] ="alienfxclient.sock";
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

    // construct client socket
    struct sockaddr_un client_addr;
    client_addr.sun_family = AF_LOCAL;
    strncpy(client_addr.sun_path, CLIENTSOCKETPATH, sizeof(client_addr.sun_path));
    client_addr.sun_path[sizeof(client_addr.sun_path) - 1] = '\0';
    /* socklen_t len_client_addr = SUN_LEN(&client_addr); */

    int client_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if(client_fd < 0) {
      exit(EXIT_FAILURE);
      plog("failed to create socket");
    }

    // construct server socket metadata, but don't open it!
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, SERVERSOCKETPATH, sizeof(server_addr.sun_path));
    server_addr.sun_path[sizeof(server_addr.sun_path) - 1] = '\0';
    socklen_t len_server_addr = SUN_LEN(&server_addr);

    // send the lil' shittles
    char the_msg[] = "hello there young padawan";
    if (sendto(client_fd, the_msg, sizeof(the_msg), 0, (struct sockaddr *) &server_addr, len_server_addr) < 0) exit(EXIT_FAILURE);

    shutdown(client_fd, 0);
}
