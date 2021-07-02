#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 40
#define QKEY    (key_t)0111

struct msgq_data {
    long type;
    char text[BUFSIZE];
};
struct msgq_data recv_data;

main()
{
   // int qid, len;
  /*
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

  */
    FILE* fp = popen("top -n 1 -b | awk '/^%Cpu/{print $2}'", "r");
    printf("%s",fp);
   /*
   if (recv_data.text == "MEMORY") {
        char tmp[2048];
        FILE* fp = popen("top -n 1 -b | awk '/^%Cpu/{print $2}'", "r");

        printf("%s", fp);
        
    }
   */ 
}