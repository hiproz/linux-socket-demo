#include "error.h"
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// global variable
char *SERVER_HOST_NAME = "127.0.0.1";

int SERVER_HOST_PORT_NO = 8080;
int client_socket;
char buffer_received[1024];

// function declaration
char *get_cpu_info();
char *get_cpu_info_v2();

// function defination
void action1() {
  char *content = get_cpu_info_v2();
  send(client_socket, content, strlen(content), 0);

  printf("send completed, size = %d\n", strlen(content));
}

void show_option() {
  printf("<1> case 1\n");
  printf("<2> case 2\n");
  printf("<3> case 3\n");
  printf("<4> case 4\n");
}
// main entrance
int main() {

  struct sockaddr_in client_addr;
  bzero(&client_addr, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(0);

  // create a socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    fail("create client socket fail");
  }

  struct sockaddr_in server_addr;
  bzero((char *)&server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;

  struct hostent *server;
  server = gethostbyname(SERVER_HOST_NAME);
  if (server == NULL) {
    fail("fail to get host name");
  }
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
        server->h_length);

  server_addr.sin_port = htons(atoi("8080"));
  socklen_t server_addr_len = sizeof(server_addr);

  // set the nonblocking mode
  int mode = 1;
  ioctl(client_socket, FIONBIO, &mode); //设为非阻塞式

  printf("connecting to %s, port=%d\n", inet_ntoa(server_addr.sin_addr.s_addr),
         server_addr.sin_port);
  if (connect(client_socket, (struct sockaddr *)&server_addr,
              server_addr_len) == -1) {
    fail("connent to server fail");
  }

  // set input nonblocking
  int flags;
  if ((flags = fcntl(STDOUT_FILENO, F_GETFL, 0)) == -1) {
    perror("fcntl F_GETFL fail:");
    exit(1);
  }
  flags |= O_NONBLOCK;
  if (fcntl(STDOUT_FILENO, F_SETFL, flags) == -1) {
    perror("fcntl F_SETFL fail:");
    exit(1);
  }

  printf("please input your choice:\n");
  show_option();

  while (1) {
    // 1. input
    char c = getchar();
    if (c != -1) {
      switch (c) {
      case '1':
        action1();
        printf("1 procedrue\n");
        break;
      case '\n':
        show_option();
        break;
      default:
        printf("invalid choice!");
        break;
      }
    } else {
      // fail("key input failed");
    }

    // 2. socket
    memset(buffer_received, '\0', 1024);

    int length_received =
        recv(client_socket, buffer_received, sizeof(buffer_received), 0);
    if (length_received <= 0) {
      // fail("receive fail");
      continue;
    }
    buffer_received[length_received] = '\0';
    printf("get bytes length: %d,content:%s\n", length_received,
           buffer_received);
  }

  close(client_socket);

  return 0;
}

char *get_cpu_info() {
  FILE *cpuinfo = fopen("/proc/cpuinfo", "rb");
  char *config = "\0";
  size_t size = 0;
  while (getdelim(&config, &size, 0, cpuinfo) != -1) {
    puts(config);
  }

  fclose(cpuinfo);

  return config;
}
char buffer[1024];
char *get_cpu_info_v2() {
  int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
  sprintf(buffer, "cpu num = %d\n", cpu_num);

  return buffer;
}
