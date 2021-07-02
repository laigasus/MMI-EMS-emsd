#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int tcp_server_open(void) {
    
    int serv_sock;
    struct sockaddr_in serv_addr =
    {
     .sin_family = AF_INET,
     .sin_addr.s_addr = htonl(INADDR_ANY),
     .sin_port = htons(5303),
    };
    

    // 1. 소켓 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) return -1;

    // 2. 바인드
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof serv_addr) < 0) return -2;

    // 3. listen
    if (listen(serv_sock, 5) < 0) return -3;

    
    return serv_sock;
}

int tcp_server_worker(int fd, char *buf) 
{
    

    printf("worker start\n");
    int i = 0;
    char* arg[4] = { "", "", "" };
    char send_buf[2048] = "";
    char ls_result[2048] = "";

    // 1. 아규먼트 토큰 분리( 여기는 \n )
    for (char* p = strtok(buf, "\n"); p; p = strtok(NULL, "\n")) {
        arg[i++] = p;
    }
    printf("토큰 분리 성공\n");
    // 2. 아규먼트 별 명령 실행
    if (strcmp(arg[0], "list") == 0) {
        printf("list 명령 실행\n");
        //char ls_result[2048] = "";
        char flist[128][128] = { 0, };
        FILE* fp = popen("ls -1t board", "r");
        int idx = 0;

        // 파일 목록 얻어오기
        fread(ls_result, sizeof ls_result, 1, fp);
       // printf("%s\n",ls_result);
        for (char* p = strtok(ls_result, "\n\r"); p; p = strtok(NULL, "\n\r")) {
            strcpy(flist[idx++], p);
        }
        pclose(fp);
        // 파일 내용 얻어오기
        for (int j = 0; j < idx; j++) {
            char fname[256], tmp[256], title[256];
            sprintf(fname, "board/%s", flist[j]);
            fp = fopen(fname, "r");
            fscanf(fp, "%[^\n]", title);
            fclose(fp);

            sprintf(tmp, "%2d %s\n", j + 1, title);
            strcat(send_buf, tmp);
        }
    }
    else if (strcmp(arg[0], "write") == 0) {
        //파일 개수 얻어오기
        FILE* fp = popen("ls -l board | wc -l", "r");
        int nfcnt;
        char fcnt[32] = "", fname[128];
        fgets(fcnt, sizeof fcnt, fp);
        printf(fcnt);
        pclose(fp);
        nfcnt = atoi(fcnt) + 1;

        //파일 생성
        sprintf(fname, "board/%d.txt", nfcnt);
        fp = fopen(fname, "w");
        fprintf(fp, "%s\n%s", arg[1], arg[2]);
        fclose(fp);
    }
    else if (strcmp(arg[0], "show") == 0) {
        //파일 이름 읽어오기
        char tmp[2048], tmp2[2048];
        FILE* fp;

        sprintf(tmp, "board/%s.txt", arg[1]);
        fp = fopen(tmp, "r");
        fscanf(fp, "%[^\n]%*c", tmp);
        fscanf(fp, "%[^\n]", tmp2);
        fclose(fp);
        sprintf(send_buf, "제목 : %s\n 내용 : %s\n", tmp, tmp2);
    }
    // 3. 전송
    //return sendto(fd, send_buf, strlen(send_buf), 0, (struct sockaddr*)paddr, sizeof(*paddr));
    printf("%s\n", ls_result);
    write(fd, send_buf, strlen(send_buf));
    close(fd);
}

int main(void) {
    // 1. 서버 생성
    int clnt_sock;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;
    int serv_sock = tcp_server_open(), addr_len, recv_len;
    char buf[2048];

    switch (serv_sock) {
    case -1: printf("소켓 생성 실패\n"); return 1;
    case -2: printf("바인드 실패\n"); return 1;
    case -3: printf("listen 실패\n"); return 1;
    }

    while (1) {
        // 2. 메세지 수신
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size); //연결수락
        printf("accept 성공\n");
        recv_len = read(clnt_sock, buf, sizeof buf);
        if (recv_len < 0) continue;
        buf[recv_len] = '\0';
        printf(buf);
        // 3. 명령 처리
        tcp_server_worker(clnt_sock,  buf);
    }
}