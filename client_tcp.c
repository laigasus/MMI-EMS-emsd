#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void send_cmd(const char *cmd)
{
    struct sockaddr_in addr =
    {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_port = htons(5303)
    };
    int addr_len = sizeof addr, recv_len;
    char buf[2048];
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if(connect(fd, (struct sockaddr*)&addr, addr_len) == -1){
        printf("Can not connect\n");
    }
    write(fd, cmd, strlen(cmd));
    recv_len = read(fd, buf, sizeof buf);
    printf("%s\n", buf);
    close(fd);
}

int main(int argc, char *argv[])
{
    char cmd[2048] = "";
    int i;
    for(i = 1; i < argc; i++)
    {
        strcat(cmd, argv[i]);
        strcat(cmd, "\n");
    }
    send_cmd(cmd);
}
