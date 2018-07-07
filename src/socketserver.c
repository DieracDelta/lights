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
#include <sys/un.h>
#include <stdarg.h>

#define DEBUG 1

const char SERVERSOCKETPATH[] = "alienfxserver.sock";
/* const char PIDPATH[] = "/var/run/alienfx"; */
/* const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid"; */
/* const char STATEFILEPATH[] = "~/.alienfx.yaml"; */
#define PIDPATH "/var/run/alienfx";
#define PIDFILEPATH "/var/run/alienfx/alienfx.pid";
#define STATEFILEPATH "~/.alienfx.yaml";

static int pid_fd;
static int on;

#define PACKET_SIZE 1024

void main(){
  // TODO better logging
  remove(SERVERSOCKETPATH);

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

  int success = bind(socket_fd, (struct sockaddr *)&socket_addr, len_socket_addr);
  if(success < 0){
    // TODO log it
    printf("failed to bind to socket with code %d", success);
    exit(EXIT_FAILURE);
  }

  // define garbage pointer in case you need to know where stuff came from
  /* struct sockaddr_un garbage; */
  /* socklen_t garbage_len = SUN_LEN(&garbage); */

  char packet[PACKET_SIZE];
  // TODO replace with while running
  while(1){
    printf("hoi\n");
    // in case you need to know where stuff came from
    /* recvfrom(socket_fd, packet, PACKET_SIZE, 0, (struct sockaddr *) &garbage, &garbage_len); */
    int size = recv(socket_fd, packet, PACKET_SIZE, 0);
    if(size < 0){
      // TODO log it
      printf("failed to receive message");
    }
    fprintf(stdout, "hoi there %s\n", packet);
  }



  shutdown(socket_fd, 0);
  remove(SERVERSOCKETPATH);
}
