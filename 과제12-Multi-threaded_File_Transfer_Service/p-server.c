#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/wait.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799

int main(void) {
    int i, s_sock, c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    char hello[] = "Hello~ I am Server!\n";
    pid_t pid;
    int wstatus;
    ssize_t check; //QnA 내용처럼 send와 recv 반환값 검사용 변수

    s_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓 열기

    int option = 1;     //SO_REUSEADDR의 옵션 값을 TRUE로 : 프로그램 종료후 다시실행때 bind()에서 오류 무시하기위해 사용
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    bzero(&server_addr,sizeof(server_addr)); //server_addr 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 동작하는 컴퓨터 IP주소 자동 할당

    if (bind(s_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("[S] Can't bind a socket"); // 바인드 안된 경우 예외처리
        exit(1);
    }

    listen(s_sock,1);
    c_addr_size = sizeof(struct sockaddr);

    for(i=0; i<3; i++) {
        printf("[S] waiting for a client..#%02d\n",i);
        c_sock = accept(s_sock, (struct sockaddr *) &client_addr, &c_addr_size);
        if (c_sock == -1) {
            perror("[S] Can't accept a connection"); //accept 예외처리
            exit(1);
        }

        pid = fork();

        if (pid < 0 ) // fork 에러
        {
            perror("[S] Fork Faild");
            return 1;
        } else if ( pid == 0 ) //자식 프로세스
        {
            close(s_sock);

            printf("[S-%d] Connected: client IP addr=%s port=%d\n",getpid(), inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            //1. say hello to client
            check = send(c_sock, hello, sizeof(hello)+1,0);
            if ( (check == -1) || (check == 0) ) {
                printf("[S-%d] Can't send message\n",getpid());
                exit(1);
            }

            printf("[S-%d] I said Hello to Client!\n",getpid());

            //2. recv msg from client
            check = recv(c_sock,buf,BUFFSIZE, 0);
            if ((check == -1) || (check == 0)) {
                printf("[S-%d] Can't receive message\n",getpid());
                exit(1);
            }        

            printf("[S-%d] Client says: %s\n",getpid(), buf);

            close(c_sock);

            return 0;
        }else //부모 프로세스
        {
            printf("[S] I open the [S-%d] at #%02d\n",pid,i);
            close(c_sock);
        }

    }
    close(s_sock);

    for(i=0; i<3; i++){
        pid = wait(&wstatus);
        printf("[S] My Child [S-%d] is finised with %d\n",pid,wstatus);
    }

    return 0;
}