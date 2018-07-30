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
#include <errno.h>
#include "alienfx_protocol.h"

#define DEBUG 1

/* const char SERVERSOCKETPATH[] = "/tmp/alienfxserver.sock"; */
/* const char CLIENTSOCKETPATH[] ="/tmp/alienfxclient.sock"; */
const char SERVERSOCKETPATH[] = "/tmp/alienfxsocket.sock";
const char CLIENTSOCKETPATH[] ="/tmp/alienfxclient.sock";
const char PIDPATH[] = "/var/run/alienfx";
const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid";
// TODO store state elsewhere
const char STATEFILEPATH[] = "~/.alienfx.yaml";

static int pid_fd;
static int on;


void send_alienfx_msg(struct alienfx_msg * packet, struct alienfx_response * resp){
  printf("ffs\n");


  int client_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
  if(client_fd < 0) {
    // failed
    return;
  }

  // construct client socket
  struct sockaddr_un client_addr;
  client_addr.sun_family = AF_LOCAL;
  strncpy(client_addr.sun_path, CLIENTSOCKETPATH, sizeof(client_addr.sun_path));
  client_addr.sun_path[sizeof(client_addr.sun_path) - 1] = '\0';
  socklen_t len_client_addr = SUN_LEN(&client_addr);

  unlink(CLIENTSOCKETPATH);
  int success = bind(client_fd, (struct sockaddr *)&client_addr, len_client_addr);
  if(success < 0){
    // TODO log it
    printf("failed to bind to socket with code %d\r\n", success);
    exit(EXIT_FAILURE);
  }

  // construct server socket metadata, but don't open it!
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SERVERSOCKETPATH, sizeof(server_addr.sun_path));
  server_addr.sun_path[sizeof(server_addr.sun_path) - 1] = '\0';
  socklen_t len_server_addr = SUN_LEN(&server_addr);

  success = connect(client_fd, (struct sockaddr *)&server_addr, len_server_addr);
  if(success < 0){
    // TODO log it
    printf("failed to bind to socket with code %d\r\n", success);
    exit(EXIT_FAILURE);
  }


  printf("yeettt\n");
  if (send(client_fd, packet, sizeof(struct alienfx_msg), 0) < 0) return;
  // TODO probably gonna get overwritten, so either malloc in here or zero it out
  printf("yeettt2\n");


  printf("yeettt3\n");
  success = recv(client_fd, resp, sizeof(struct alienfx_response), MSG_DONTWAIT);
  if(success < 0){
    printf("error!  %d\r\n\r\n", success);
    printf("error: %d, reason: %s\r\n\r\n", errno, strerror(errno));
  }
  printf("yeettt4\n");

  shutdown(client_fd, 0);
}

uint64_t get_info(uint8_t profile_index, uint16_t region, uint8_t get_flag){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = GET;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t) (region >> 8) & 0xff ;
  packet->args[1] = (uint8_t) region & 0xff;
  packet->args[2] = get_flag;

  send_alienfx_msg(packet, resp);

  free(packet);
  free(resp);
  return resp->r_val;
}

// TODO fix this copy pasta
int set_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = SET_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t) (region >> 8) & 0xff ;
  packet->args[1] = (uint8_t) region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  send_alienfx_msg(packet, resp);

  int rval = resp->error;

  free(packet);
  free(resp);

  return rval;
}

int increment_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = INCREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t) (region >> 8) & 0xff ;
  packet->args[1] = (uint8_t) region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  send_alienfx_msg(packet, resp);

  int rval = resp->error;

  free(packet);
  free(resp);

  return rval;
}

int decrement_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = DECREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t) (region >> 8) & 0xff ;
  packet->args[1] = (uint8_t) region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  printf("sending...\n");
  send_alienfx_msg(packet, resp);
  printf("sent!...\n");

  int rval = resp->error;

  free(packet);
  free(resp);

  return rval;
}

int toggle_pause(uint8_t profile_index, uint16_t region){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = DECREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t) (region >> 8) & 0xff ;
  packet->args[1] = (uint8_t) region & 0xff;

  send_alienfx_msg(packet, resp);

  int rval = resp->error;

  free(packet);
  free(resp);

  return rval;
}

int inc_profile(){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = INCREMENT_PROFILE;

  int rval = resp->error;

  send_alienfx_msg(packet, resp);

  free(packet);
  free(resp);

  return rval;
}

int dec_profile(){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = DECREMENT_PROFILE;

  int rval = resp->error;

  send_alienfx_msg(packet, resp);

  free(packet);
  free(resp);

  return rval;
}

int set_profile(uint8_t profile_index){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = SET_PROFILE;
  packet->profile_index = profile_index;

  int rval = resp->error;

  send_alienfx_msg(packet, resp);

  free(packet);
  free(resp);

  return rval;
}

uint8_t get_cur_profile_index(){
  struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg));
  struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));

  packet->OP = GET;

  int rval = resp->error;

  send_alienfx_msg(packet, resp);

  free(packet);
  free(resp);

  return rval;
}
