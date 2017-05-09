#include "error.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int PORT_NO = 8080;
int new_server_socket = NULL;
char buffer_received[1024];

int action1() {
  send(new_server_socket, buffer_received, strlen(buffer_received), 0);
}
void show_option() {
  printf("<1> case 1\n");
  printf("<2> case 2\n");
  printf("<3> case 3\n");
  printf("<4> case 4\n");
}
int main() {
  int socket_fd, client_fd, port_no;
  char buffer[256];
  struct sockaddr_in server_addr, client_addr;

  // create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    fail("create socket fail");
  }

  // configure the server
  port_no = PORT_NO;
  bzero((char *)&server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_no);

  int opt = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  int mode = 1;
  ioctl(socket_fd, FIONBIO, &mode); //设为非阻塞式
  if (bind(socket_fd, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr)) < 0) {
    fail("bind socket fail");
  }

  // listen the socket
  if (listen(socket_fd, 500)) {
    fail("listen socket fail");
  }

  printf("the server started...\n");
  printf("listening: addr=%s, port=%d\n",
         inet_ntoa(server_addr.sin_addr.s_addr), port_no);

  printf("waiting for client...\n");

  socklen_t length = sizeof(struct sockaddr_in);

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
    if (!new_server_socket) {
      new_server_socket =
          accept(socket_fd, (struct sockaddr *)&client_addr, &length);

      if (new_server_socket < 0) {
        // fail("accept fail");
        continue;
      } else {
        printf("accept client %s,socket:%d\n", inet_ntoa(client_addr.sin_addr),
               new_server_socket);
      }
    }

    memset(buffer_received, '\0', 1024);

    int length_received =
        recv(new_server_socket, buffer_received, sizeof(buffer_received), 0);
    if (length_received <= 0) {
      // fail("receive fail");
      new_server_socket = NULL;
      continue;
    }

    buffer_received[length_received] = '\0';
    printf("get bytes length: %d,content:%s\n", length_received,
           buffer_received);

    send(new_server_socket, buffer_received, strlen(buffer_received), 0);
  }
  return 0;
}
