#include "alienfx_protocol.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#define DEBUG 1

/* const char SERVERSOCKETPATH[] = "/tmp/alienfxserver.sock"; */
/* const char CLIENTSOCKETPATH[] ="/tmp/alienfxclient.sock"; */
const char SERVERSOCKETPATH[] = "/tmp/alienfxsocket.sock";
// TODO store state elsewhere
const char STATEFILEPATH[] = "~/.alienfx.yaml";

int do_add(int a, int b) { return a + b; }

void send_alienfx_msg(struct alienfx_msg *packet) {

  int client_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
  if (client_fd < 0) {
    // failed
    return;
  }

  // construct client socket
  /* struct sockaddr_un client_addr; */
  /* client_addr.sun_family = AF_LOCAL; */
  /* strncpy(client_addr.sun_path, CLIENTSOCKETPATH,
   * sizeof(client_addr.sun_path)); */
  /* client_addr.sun_path[sizeof(client_addr.sun_path) - 1] = '\0'; */
  /* socklen_t len_client_addr = SUN_LEN(&client_addr); */

  // construct server socket metadata, but don't open it!
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, SERVERSOCKETPATH, sizeof(server_addr.sun_path));
  server_addr.sun_path[sizeof(server_addr.sun_path) - 1] = '\0';
  socklen_t len_server_addr = SUN_LEN(&server_addr);

  if (sendto(client_fd, packet, sizeof(struct alienfx_msg), 0,
             (struct sockaddr *)&server_addr, len_server_addr) < 0)
    return;

  shutdown(client_fd, 0);
}

// FIXME fix this
/* uint64_t get_info(uint8_t profile_index, uint16_t region, uint8_t get_flag){
 */
/*   struct alienfx_msg * packet = malloc(sizeof(struct alienfx_msg)); */
/*   struct alienfx_response * resp = malloc(sizeof(struct alienfx_response));
 */

/*   packet->OP = GET; */
/*   packet->profile_index = profile_index; */
/*   packet->args[0] = (uint8_t) (region >> 8) & 0xff ; */
/*   packet->args[1] = (uint8_t) region & 0xff; */
/*   packet->args[2] = get_flag; */

/*   send_alienfx_msg(packet, resp); */

/*   free(packet); */
/*   free(resp); */
/*   return resp->r_val; */
/* } */

// TODO fix this copy pasta
void set_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags,
                uint8_t red, uint8_t blue, uint8_t green) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = SET_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  send_alienfx_msg(packet);

  free(packet);
}

void increment_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags,
                      uint8_t red, uint8_t blue, uint8_t green) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = INCREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  send_alienfx_msg(packet);
  free(packet);
}

void decrement_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags,
                      uint8_t red, uint8_t blue, uint8_t green) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = DECREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = rgb_flags;
  packet->args[3] = red;
  packet->args[4] = green;
  packet->args[5] = blue;

  send_alienfx_msg(packet);

  free(packet);
}

void set_freq(uint8_t profile_index, uint16_t region, uint16_t freq) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));
  packet->OP = SET_FREQ;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = (uint8_t)(freq >> 8) & 0xff;
  packet->args[3] = (uint8_t)freq & 0xff;

  send_alienfx_msg(packet);

  free(packet);
}

void increment_freq(uint8_t profile_index, uint16_t region, uint16_t freq) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));
  packet->OP = SET_FREQ;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = (uint8_t)(freq >> 8) & 0xff;
  packet->args[3] = (uint8_t)freq & 0xff;

  send_alienfx_msg(packet);

  free(packet);
}

void decrement_freq(uint8_t profile_index, uint16_t region, uint16_t freq) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));
  packet->OP = SET_FREQ;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;
  packet->args[2] = (uint8_t)(freq >> 8) & 0xff;
  packet->args[3] = (uint8_t)freq & 0xff;

  send_alienfx_msg(packet);

  free(packet);
}

void toggle_pause(uint8_t profile_index, uint16_t region) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = DECREMENT_COLOR;
  packet->profile_index = profile_index;
  packet->args[0] = (uint8_t)(region >> 8) & 0xff;
  packet->args[1] = (uint8_t)region & 0xff;

  send_alienfx_msg(packet);

  free(packet);
}

void inc_profile() {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = INCREMENT_PROFILE;

  send_alienfx_msg(packet);

  free(packet);
}

void dec_profile() {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = DECREMENT_PROFILE;

  send_alienfx_msg(packet);

  free(packet);
}

void set_profile(uint8_t profile_index) {
  struct alienfx_msg *packet = malloc(sizeof(struct alienfx_msg));

  packet->OP = SET_PROFILE;
  packet->profile_index = profile_index;

  send_alienfx_msg(packet);

  free(packet);
}
