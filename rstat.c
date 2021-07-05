#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define BUFSIZE 40
#define QKEY (key_t)0111

typedef struct msgq_data {
  long type;
  char text[BUFSIZE];
} Message;

void main() {
  int qid, len;
  char tmp[2048];
  char fcnt[BUFSIZE] = "";
  FILE* fp;

  printf("msgq 생성\n");
  if ((qid = msgget(QKEY, IPC_CREAT | 0666)) == -1) {
    perror("msgget failed");
    exit(1);
  }
  while (1) {
    Message recv_data, send_data;
    memset(&recv_data, 0x00, sizeof(recv_data));

    if ((len = msgrcv(qid, &recv_data, BUFSIZE, 0, 0)) == -1) {
      perror("msgrcv failed");
      exit(1);
    }

    printf("%s = 받은 문자값\n", recv_data.text);
    printf("%s\n", recv_data.text);

    if (strcmp(recv_data.text, "CPU") == 0) {
      fp = popen("top -n 1 -b | awk '/^%Cpu/{print $2}'", "r");
    } else if (strcmp(recv_data.text, "MEMORY") == 0) {
      fp = popen("free | grep Mem | awk '{print $4/$3 * 100.0}'", "r");
    } else if (strcmp(recv_data.text, "DISK") == 0) {
      fp = popen("df|tail -1|tr -s ' '|cut -d ' ' -f5", "r");
    }
    fgets(fcnt, sizeof fcnt, fp);
    printf("%s", fcnt);
    send_data.type = 1;
    sprintf(send_data.text, "%s", fcnt);
    msgsnd(qid, &send_data, strlen(send_data.text), 0);
  }
}