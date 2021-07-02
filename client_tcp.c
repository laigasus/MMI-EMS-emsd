#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
//"192.168.1.128", "192.168.1.145"
// void change_addr_info(struct sockaddr_in addr, const char *ip) {

//   addr.sin_addr.s_addr = inet_addr(ip);
//   addr.sin_port = htons(5303);
// }

void send_cmd(const char *cmd) {
  int conn_repeat = 0;
  char *ip_list[] = {"192.168.227.23", "192.168.227.92"};

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_addr.s_addr = inet_addr(ip_list[conn_repeat]),
                             .sin_port = htons(5303)};
  int addr_len = sizeof addr, recv_len;
  char buf[2048];
  int fd = socket(PF_INET, SOCK_STREAM, 0);
  bool conn_flag = false;

  do {
    if (connect(fd, (struct sockaddr *)&addr, addr_len) == -1) {
      conn_repeat++;
      printf("Change Connect..<%s>\t", ip_list[conn_repeat]);
      printf("repeat: %d\n", conn_repeat);
      addr.sin_addr.s_addr = inet_addr(ip_list[conn_repeat]);
      // change_addr_info(addr, ip_list[conn_repeat]);
      conn_flag = true;
    } else {
      conn_flag = false;
    }
  } while (conn_flag);

  write(fd, cmd, strlen(cmd));
  recv_len = read(fd, buf, sizeof buf);
  printf("%s\n", buf);
  close(fd);
}

int main(int argc, char *argv[]) {
  char cmd[2048] = "";
  int i;
  for (i = 1; i < argc; i++) {
    strcat(cmd, argv[i]);
    strcat(cmd, "\n");
  }
  send_cmd(cmd);
}