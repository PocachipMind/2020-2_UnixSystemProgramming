#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799

int main(void) {
    int c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    char hello[] = "Hi~ I am Client!!\n";
    ssize_t check; //QnA 내용처럼 send와 recv 반환값 검사용 변수

    c_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓열기

    bzero(&server_addr, sizeof(server_addr)); // server_addr 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    printf("[C] Connecting...\n");

    if (connect(c_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("[C] Can't connect to a Server"); // 커넥트 예외처리
        exit(1);
    }

    printf("[C] Connected!\n");

    //1. recv msg from server (maybe it's "hello")
    check = recv(c_sock, buf, BUFFSIZE, 0);
    if( (check == -1) || (check == 0)) {
        perror("[C] Can't receive message");
        exit(1);
    }

    printf("[C] Server says: %s\n", buf);

    //2. say hi to server
    check = send(c_sock, hello, sizeof(hello)+1, 0);
    if ( (check == -1) || (check == 0)) {
        perror("[C] Can't send message");
        exit(1);
    }

    printf("[C] I said Hi to Server!!\n");
    

    //printf("[C] I am going to sleep...\n");
    //sleep(10);

    close(c_sock);

    return 0;

}