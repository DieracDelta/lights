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


const char PIDPATH[] = "/var/run/alienfx";
const char PIDFILEPATH[] = "/var/run/alienfx/alienfx.pid";
const char STATEFILEPATH[] = "~/.alienfx.yaml";

static int pid_fd;
static int on;

void plog(const char * msg);
void plog(const char * msg){
  static int counter = 0;
  // TODO add more in-depth logging
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX %d: %s \n", counter++, msg);
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
 success:
  plog("successfully closed pid file");

  plog("EXITING");
  return;
}

// inspired by a bazillion sources, all of which just said "fork twice my frand"

void main(){

  // BOOKKEEPING
  printf("made it 9\n");
  pid_t pid = fork();
  printf("made it 10\n");

  // exit parent
  if(pid < 0) exit(EXIT_FAILURE);
  printf("made it 8\n");
  // TODO exit or _exit?
  if(pid > 0) exit(EXIT_SUCCESS);
  printf("made it 7\n");

  // be your own session
  if(setsid() < 0) exit(EXIT_FAILURE);
  printf("made it 6\n");

  int second_pid = fork();
  printf("made it 5\n");

  // exit child, leaving grandchild
  if(second_pid < 0) exit(EXIT_FAILURE);
  printf("made it 4\n");
  if(second_pid > 0) exit(EXIT_SUCCESS);
  printf("made it 3\n");


  // switch cwd to root to allow for unmounting 
  if(chdir("/") < 0) exit(EXIT_FAILURE);
  printf("made it 2\n");

  // child has full access to files
  umask(0);
  printf("made it 1\n");

  // alternative to remove stdin stdout stderr fds
  /* close(STDIN_FILENO); */
  /* close(STDOUT_FILENO); */
  /* close(STDERR_FILENO); */

  // close fds
  /* for(int fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--){ */
  /*   close(fd); */
  /* } */

  // write pid to PIDFILEPATH
  // initially st had a zero in it but that's equivalent to null soo
  printf("made it 14\r\n");
  struct stat st = {};
  if(stat(PIDPATH, &st) < 0) {
    printf("made it 16\r\n");
    mkdir(PIDPATH, 0755);
  }

  printf("made it 15\r\n");
  pid_fd = open(PIDFILEPATH, O_RDWR|O_CREAT, 0755);
  printf("made it 16 %d\r\n", pid_fd);
  if(pid_fd < 0) exit(EXIT_FAILURE);
  printf("made it 11\r\n");

  // F_TLOCK because we don't want blocking, we want error if the file is already locked
  if(lockf(pid_fd, F_TLOCK, 0) < 0) exit(EXIT_FAILURE);
  printf("made it 12\r\n");
  char str[69];
  sprintf(str, "%d", getpid());
  if (write(pid_fd, str, strlen(str)) < 0) exit(EXIT_FAILURE);
  printf("made it 13\r\n");


  // open up logs; we're not taking input
  // TODO we're not taking input right
  stdin = fopen("/dev/null", "r");
  stdout = fopen("/dev/null", "r");
  stderr = fopen("/dev/null", "r");

  // catch all the signals
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGQUIT, sig_handler);

  // docs: https://www.gnu.org/software/libc/manual/html_node/openlog.html
  // TODO docs say that I *do NOT* have to open daemon
  plog("ALIENFX has started successfully");

  on = 1;


  while(on){
    plog("HII BITCH");
    sleep(100);
  }
}

