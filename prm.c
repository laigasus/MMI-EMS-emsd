#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 40
#define QKEY (key_t)1112

typedef struct msgq_data {
  long type;
  char text[BUFSIZE];
} Message;

char* get_pipe_result(char* query, char* type) {
  FILE* fp;
  char fcnt[BUFSIZE] = "";
  fp = popen(query, type);
  return fgets(fcnt, sizeof fcnt, fp);
}

char* get_ip() {
  int fd;
  struct ifreq ifr;

  char iface[] = "enp1s0";

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  // ipv4 형태의 주소 반환
  ifr.ifr_addr.sa_family = AF_INET;

  // ifreq 구조체에서 인터페이스 이름 받아냄
  strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);

  return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

void main() {
  int qid, len;
  char fcnt[BUFSIZE] = "";
  printf("prm is running\n");

  if ((qid = msgget(QKEY, IPC_CREAT | 0666)) == -1) {
    perror("msgget failed");
    exit(1);
  }

  char* ip = get_ip();
  char* pname = get_pipe_result(
      "ps -ef | grep ./rstat | head -1 | tr -s ' ' | cut -d ' ' -f8", "r");
  int pid = getpid();

  while (1) {
    Message recv_data, send_data;
    memset(&recv_data, 0x00, sizeof(recv_data));

    if ((len = msgrcv(qid, &recv_data, BUFSIZE, 0, 0)) == -1) {
      perror("msgrcv failed");
      exit(1);
    }
    printf("메세지 수신>>%s", recv_data.text);

    send_data.type = 1;

    if (!strcmp(recv_data.text, "ACT")) {
      if (!strcmp(pname, "./rstat")) {
        sprintf(send_data.text, "rstat is  already running\nip>%s\npid>%d", ip,
                pid);
      } else {
        popen("./rstat", "r");
        sprintf(send_data.text, "turn on rstat\nip>%s\npid>%d", ip, pid);
      }
    } else if (!strcmp(recv_data.text, "SBY")) {
      popen("killall -9 ./rstat", "r");
      if (!strcmp(pname, "./rstat")) {
        sprintf(send_data.text, "can't kill rstat\nip>%s\npid>%d", ip, pid);
      } else {
        sprintf(send_data.text, "rstat killed!!\n");
      }
    }
    msgsnd(qid, &send_data, strlen(send_data.text), 0);
    printf("메세지 전송>>%s", send_data.text);
  }
}

// agtd 두개의 rstat은 동일한 이름과 역할을 수행한다.
// 두개의 rstat을 구분해서 파악하려면 프로세스 이름이 아닌 pid로 구분을 한다
// rstat 프로세스 상태를 확인하려면 getpid()를 사용한다.
// 사전에 rstat에서 getpid()로 실행하여 메세지큐로 prm에 전달되어 있어야 한다
// rstat와 prm은 독립적으로 실행시키는 것이 아닌 agtd가 활성화 되었을때
// 가동한다(자원 낭비 방지) popen으로 awk를 사용하여 결과 fcnt출력값이 없으면
// killed, 있으면 running을 agtd에게 전달한다.
