#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sts/ipc.h>

// 192.168.1.128, 192.168.1.145

int mmi_socket_open(void) {
	int mmi_server_socket;
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_ANY),
		.sin_port = htons(9000)
	};
	mmi_server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (mmi_server_socket < 0) return -1;

	if (bind(mmi_server_socket, (struct sockaddr*)&serv_addr), sizeof serv_addr) < 0) return -2;

	if (listen(mmi_server_socket, 5) < 0) return -3;

	return mmi_server_socket;
}

int stdby_socket_open(void) {
	int stdby_server_socket;
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_ANY),
		.sin_port = htons(9001)
	};
	stdby_server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (stdby_server_socket < 0) return -1;

	if (bind(stdby_server_socket, (struct sockaddr*)&serv_addr), sizeof serv_addr) < 0) return -2;

	if (listen(stdby_server_socket, 5) < 0) return -3;

	return stdby_server_socket;
}


int mmi_server_worker(int clnt_sock, char *buf) {
	int i = 0;
	char* arg[4] = { "", "", "" };
	char send_buf[2048] = "";

	int qid,len;
	
	// 아규먼트 토큰 분리
	for (char* p = strtok(buf, "\n"); p; p = strtok(NULL, "\n")) {
		arg[i++] = p;
	}
	printf("토큰 분리 성공\n");

	struct msgq_data {
		long type;
		char text[2048];
	};

	if ((qid = msgget((key_t)0111, IPC_CREAT | 0666)) == -1) {
		printf("메시지 큐 생성 실패\n");
	}


	

	//아규먼트 별 명령 실행
	if (strcmp(arg[0], "DIS-RESOURCE")) {
		if (strcmp(arg[1], "MEMORY")) {
			struct msgq_data send_data = { 1, arg[1] };
			struct msgq_data recv_data;
			if (msgsnd(qid, &send_data, strlen(send_data.text), 0) == -1) {
				printf("메시지 큐 전송 실패\n");
			}
			if ((len = msgrcv(qid, &recv_data, 100, 0, 0)) == -1)
			{
				printf("메시지 큐 수신 실패\n");
			}
			send_buf = recv_data.text;
			if (msgctl(qid, IPC_RMID, 0) == -1) {
				printf(("msgctl 실패\n");
			}
		}
		else if (strcmp(arg[1], "DISK")) {
			struct msgq_data send_data = { 1, arg[1] };
			struct msgq_data recv_data;
			if (msgsnd(qid, &send_data, strlen(send_data.text), 0) == -1) {
				printf("메시지 큐 전송 실패\n");
			}
			if ((len = msgrcv(qid, &recv_data, 100, 0, 0)) == -1)
			{
				printf("메시지 큐 수신 실패\n");
			}
			send_buf = recv_data.text;
			if (msgctl(qid, IPC_RMID, 0) == -1) {
				printf(("msgctl 실패\n");
			}
		}
		else if (strcmp(arg[1], "CPU")) {
			struct msgq_data send_data = { 1, arg[1] };
			struct msgq_data recv_data;
			if (msgsnd(qid, &send_data, strlen(send_data.text), 0) == -1) {
				printf("메시지 큐 전송 실패\n");
			}
			if ((len = msgrcv(qid, &recv_data, 100, 0, 0)) == -1)
			{
				printf("메시지 큐 수신 실패\n");
			}
			send_buf = recv_data.text;
			if (msgctl(qid, IPC_RMID, 0) == -1) {
				printf(("msgctl 실패\n");
			}
		}
		else {
			send_buf = "잘못된 명령어입니다.\n";
		}

	}
	else if (strcmp(arg[0], "DIS-SW-STS")) {

	}
	write(clnt_sock, send_buf, strlen(send_buf));
	close(clnt_sock);
}

int main(void) {
	// 서버 오픈, accept에 사용 할 변수 선언, read에 사용 할 변수 선언
	int mmi_client_socket;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	int mmi_server_socket = mmi_socket_open(), recv_len; // 서버 오픈 함수
	char buf[2048];

	switch (mmi_server_socket)
	{
	case -1: printf("소켓 생성 실패\n"); return 1;
	case -2: printf("바인드 실패\n"); return 1;
	case -3: printf("listen 실패\n"); return 1;
	}

	while (1) {
		clnt_addr_size = sizeof(clnt_addr);
		mmi_client_socket = accept(mmi_server_socket, (struct sockaddr*)&clnt_addr_size, &clnt_addr_size);
		printf("mmi 연결 성공\n");
		recv_len = read(mmi_client_socket, buf, sizeof buf);
		if (recv_len < 0) continue;
		buf[recv_len] = '\0';
		printf(buf);

		mmi_server_worker(mmi_client_socket, buf); //받은 명령어 실행 코드


	}
}

