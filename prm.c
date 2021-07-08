#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define BUFSIZE 40
#define QKEY (key_t)0112

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

        printf("받은 문자값 = %s\n", recv_data.text);

        if (!strcmp(recv_data.text, "ACT")) {
            printf("popen 실행\n");
            fp = popen("ps -ef | grep ./rstat | head -1 | tr -s ' ' | cut -d ' ' -f8", "r");
        }
        else if (!strcmp(recv_data.text, "SBY")) {
            //fp = popen("pgrep rstat", "r");
        }
        fgets(fcnt, sizeof fcnt, fp);
        printf("fcnt값 = %s", fcnt);
        send_data.type = 1;
        sprintf(send_data.text, "%s", fcnt);
        msgsnd(qid, &send_data, strlen(send_data.text), 0);
    }
}