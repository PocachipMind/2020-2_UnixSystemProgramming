#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 4096

int main(int argc, char* argv[]) {

    if(argc != 4) {
        printf("< Usage: %s IP_to_access PortNumber file >\n", argv[0]);
        return 1;
    }
    
    int c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    char saveFname[10];
    FILE *wfp;
    ssize_t check;
    int Fsize;

    c_sock = socket(AF_INET, SOCK_STREAM, 0); //소켓열기

    bzero(&server_addr, sizeof(server_addr)); // server_addr 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // 입력받은 인자로 포트번호 입력 (check)
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // 입력받은 인자로 ip 입력 (check)


    if (connect(c_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("[C] Can't connect to a Server"); // 커넥트 예외처리
        exit(1);
    }


    //전송 받을 파일이름 sever에 전달
    check = send(c_sock ,argv[3], sizeof(argv[3]), 0);
    if ((check == -1) || (check == 0)) {
        perror("[C] Can't send file name\n");
        exit(1);
    }

    saveFname[0] = 'C';
    strncpy(saveFname+1,argv[3],9);
    saveFname[9] = '\0';

    //응답받기
    check = recv(c_sock,buf,BUFFSIZE, 0);
    if ((check == -1) || (check == 0)) {
        perror("[C] Can't send file name\n");
        exit(1);
    }

    if(buf[0] == 'Y' && buf[1] == '\0')
    {
        wfp = fopen(saveFname,"wb");
        if (wfp == NULL){
            perror("[C] Can't File open\n");
            exit(1);
        }
        
        while(1)
        {
            memset(buf,0,BUFFSIZE);

            if ((Fsize = recv(c_sock, buf, BUFFSIZE, 0)) == -1) {
                perror("[C] Can't receive file data.\n");
                exit(1);
            }
            if (Fsize == 0)
            {
                break;
            }
            fwrite(buf,1,Fsize,wfp);

        }

        printf("[C] 파일 수신을 완료하였습니다.\n");
        fclose(wfp);

    } else if(buf[0] == 'N' && buf[1] == '\0')
    {
        printf("[C] 파일이 없으므로 수신하지 않습니다.\n");
    }

    close(c_sock);

    return 0;
}