#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CMD_FUNC 3
// 192.168.1.128, 192.168.1.145

typedef struct msgq_data {
  long type;
  char text[2048];
} Message;

int socket_open(char* target) {
  int sock, target_port;

  if (!strcmp(target, "mmi")) {
    target_port = 9000;
  }

  struct sockaddr_in serv_addr = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_ANY),
                                  .sin_port = htons(target_port)};

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) return -1;

  if (bind(sock, (struct sockaddr*)&serv_addr, sizeof serv_addr) < 0) return -2;

  if (listen(sock, 5) < 0) return -3;

  return sock;
}

int mmi_server_worker(int clnt_sock, char* buf) {
  int i = 0;
  bool flag = false;
  char* arg[3] = {"", "", ""};
  char send_buf[2048] = "";
  char* cmd_list[CMD_FUNC] = {"MEMORY", "DISK", "CPU"};
  int rstat_queue, prm_queue;

  // 아규먼트 토큰 분리
  for (char* p = strtok(buf, "\n"); p; p = strtok(NULL, "\n")) {
    arg[i++] = p;
  }

  Message send_data = {1L, *arg[1]};
  Message recv_data;
  memset(&recv_data, 0x00, sizeof(recv_data));

  //아규먼트 별 명령 실행
  if (!strcmp(arg[0], "DIS-RESOURCE")) {
    i = 0;
    do {
      if (!strcmp(arg[1], cmd_list[i])) {
        flag = true;
        break;
      }
      i++;
    } while (i < CMD_FUNC);
    if (flag) {
      if ((rstat_queue = msgget((key_t)0111, IPC_CREAT | 0666)) == -1) {
        perror("메시지 큐 생성 실패\n");
      }
      sprintf(send_data.text, "%s", arg[1]);
      if (msgsnd(rstat_queue, &send_data, strlen(send_data.text), 0) == -1) {
        perror("메시지 큐 전송 실패\n");
      }
      if ((msgrcv(rstat_queue, &recv_data, 100, 0, 0)) == -1) {
        perror("메시지 큐 수신 실패\n");
      }
      sprintf(send_buf, "%s", recv_data.text);
    } else {
      sprintf(send_buf, "%s", "명령어 잘못 입력\n");
    }

  } else if (!strcmp(arg[0], "DIS-SW-STS")) {
    if ((prm_queue = msgget((key_t)1112, IPC_CREAT | 0666)) == -1) {
      perror("메시지 큐 생성 실패\n");
    }
    sprintf(send_data.text, "%s", arg[1]);
    if (msgsnd(prm_queue, &send_data, strlen(send_data.text), 0) == -1) {
      perror("메시지 큐 전송 실패\n");
    }
    if (msgrcv(prm_queue, &recv_data, 100, 0, 0) == -1) {
      perror("메시지 큐 수신 실패\n");
    }
    sprintf(send_buf, "%s", recv_data.text);
  }
  write(clnt_sock, send_buf, strlen(send_buf));
  close(clnt_sock);
}

void main() {
  // 서버 오픈, accept에 사용 할 변수 선언, read에 사용 할 변수 선언
  int mmi_client_socket;
  struct sockaddr_in clnt_addr;
  int clnt_addr_size;
  int mmi_server_socket = socket_open("mmi"), recv_len;  // 서버 오픈 함수
  char buf[2048];

  switch (mmi_server_socket) {
    case -1:
      perror("소켓 생성 실패\n");
      exit(-1);
    case -2:
      perror("바인드 실패\n");
      exit(-2);
    case -3:
      perror("listen 실패\n");
      exit(-3);
  }

  while (1) {
    clnt_addr_size = sizeof(clnt_addr);
    mmi_client_socket = accept(mmi_server_socket, (struct sockaddr*)&clnt_addr,
                               &clnt_addr_size);
    printf("mmi 연결 성공\n");
    recv_len = read(mmi_client_socket, buf, sizeof buf);
    if (recv_len < 0) continue;
    buf[recv_len] = '\0';
    printf("%s\n", buf);

    mmi_server_worker(mmi_client_socket, buf);  //받은 명령어 실행 코드
  }
}
