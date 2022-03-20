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

    c_sock = socket(AF_INET, SOCK_STREAM, 0); //소켓열기

    bzero(&server_addr, sizeof(server_addr)); // server_addr 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // 입력받은 인자로 포트번호 입력 (check)
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // 입력받은 인자로 ip 입력 (check)


    if (connect(c_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("[C] Can't connect to a Server"); // 커넥트 예외처리
        exit(1);
    }


    strcpy(buf,argv[3]); // 다운 받을 파일 이름을 while 문돌기전에도 제대로 입력받기위해 옮김

    //1. 전송 받을 파일이름 sever에 전달
    if (send(c_sock ,argv[3], sizeof(argv[3]), 0) == -1) {
        perror("[C] Can't send file name\n");
        exit(1);
    }

    saveFname[0] = 'C';

    while(1){
        strncpy(saveFname+1,buf,9);
        saveFname[9] = '\0'; // buf에 saveFname 배열보다 더 많은 문자있을때 예외처리

        //4. 보낸 파일이름이 서버가 갖고있는지 응답받음
        if (recv(c_sock, buf, BUFFSIZE, 0) == -1) {
            perror("[C] Can't receive message about having file.\n");
            exit(1);
        }

        //4.1 파일 없다 응답 받음
        if (buf[0] == 'N') //서버로 부터 응답으로 그 파일이 없음을 나타냄.문자열비교 함수를 쓸 수 있으나, 코드가 길어져서 버퍼0번항목만 비교함.(strcmp)
        {
            printf("[C] Server dont't have file.\n");
            printf("[C] 다른 파일이름을 입력하시겠습니까? Y/N : \n");
            int check = -1;

            while(check == -1){
                scanf("%c",&buf[0]);
                if(buf[0] != 'Y' && buf[0] != 'N'){
                    printf("[C] 응답은 Y또는 N 한글자로만 해주십시요.\n");
                }else{
                    check = 0;
                }
            }
        }else if(buf[0] == 'Y') //4.2 파일이 있다 응답받음
        {
            printf("[C] Server have file.\n");
            break;
        }else{
            perror("[C] 코드상으로 잘못된 정보가 들어왔습니다.\n");
            exit(1);
        }

        if ( buf[0] == 'N' ){ //4.1.1 파일 안받는다고 메세지줌

            if (send(c_sock ,buf, BUFFSIZE, 0) == -1) {
                perror("[C] Can't send message");
                exit(1);
            }

            printf("파일 전송을 하지 않고 종료합니다.\n");
            close(c_sock);

            return 0;
        }else if( buf[0] == 'Y'){//4.1.2 파일 이름 재전송
            printf("파일 이름을 입력하세요 : \n");
            scanf("%s",&buf[0]);
            if (send(c_sock ,buf, BUFFSIZE, 0) == -1) {
                perror("[C] Can't send message");
                exit(1);
            }
        }
    }

    wfp = fopen(saveFname,"w");
    if (wfp == NULL){
        perror("[C] Can't File open\n");
        exit(1);
    }

    int Fsize;
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
    close(c_sock);

    return 0;
}