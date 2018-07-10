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
#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

#define DEBUG 1

/* const char SERVERSOCKETPATH[] = "/tmp/alienfxserver.sock"; */
/* const char CLIENTSOCKETPATH[] ="/tmp/alienfxclient.sock"; */
const char SERVERSOCKETPATH[] = "/tmp/alienfxsocket.sock";
const char CLIENTSOCKETPATH[] ="alienfxclient.sock";
const char PIDPATH[] = "/var/run/alienfx";
const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid";
// TOOD store state elsewhere
const char STATEFILEPATH[] = "~/.alienfx.yaml";

static int pid_fd;
static int on;


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

    struct alienfx * packet = malloc(sizeof(struct alienfx_msg));

    if (sendto(client_fd, packet, sizeof(struct alienfx_msg), 0, (struct sockaddr *) &server_addr, len_server_addr) < 0) exit(EXIT_FAILURE);

    shutdown(client_fd, 0);

    free(packet);
}
