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

  Message recv_data, send_data;

  if ((qid = msgget(QKEY, IPC_CREAT | 0666)) == -1) {
    perror("msgget failed");
    exit(1);
  }
  if ((len = msgrcv(qid, &recv_data, BUFSIZE, 0, 0)) == -1) {
    perror("msgrcv failed");
    exit(1);
  }
  if (msgctl(qid, IPC_RMID, 0) == -1) {
    perror("msgctl failed");
    exit(1);
  }

  printf("%s\n", recv_data.text);

  if (recv_data.text == "CPU") {
    FILE* fp = popen("top -n 1 -b | awk '/^%Cpu/{print $2}'", "r");

    fgets(fcnt, sizeof fcnt, fp);
    printf("%s", fcnt);

  } else if (strcmp(recv_data.text, "MEMORY") == 0) {
    FILE* fp = popen("free | grep Mem | awk '{print $4/$3 * 100.0}'", "r");
    fgets(fcnt, sizeof fcnt, fp);
    printf("%s", fcnt);
    send_data.type = 1;
    sprintf(send_data.text, "%s", fcnt);
    msgsnd(qid, &send_data, strlen(send_data.text), 0);
    if (msgctl(qid, IPC_RMID, 0) == -1) {
      perror("msgctl failed");
      exit(1);
    }
  } else if (recv_data.text == "DISK") {
    FILE* fp = popen("df|tail -1|tr -s ' '|cut -d ' ' -f5", "r");
    fgets(fcnt, sizeof fcnt, fp);
    printf("%s", fcnt);
  }
}