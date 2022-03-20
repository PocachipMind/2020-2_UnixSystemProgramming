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
    int s_sock, c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    FILE *rfp;

    s_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓열기

    int option = 1; //프로그램 종료후 다시 실행시켰을때, 오류 무시하기위해 사용
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    bzero(&server_addr, sizeof(server_addr)); // server_addr 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 동작하는 컴퓨터의 IP주소 자동 할당!

    if (bind(s_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
        perror("[S] Can't bind a socket"); // 바인드 안된경우 예외처리
        exit(1);
    }

    printf("[S] I'm waiting for client's connect! My port is %d.\n",SERVERPORT);

    listen(s_sock,1); //1명의 요청 받는중

    c_addr_size = sizeof(struct sockaddr);
    c_sock = accept(s_sock, (struct sockaddr *) &client_addr, &c_addr_size);
    if(c_sock == -1){
        perror("[S] Can't accept a connection"); //accept 예외처리
        exit(1);
    }

    printf("[S] Connected: client IP addr=%s port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));



    //2. 클라이언트에게 파일 이름 받아옴.
    if (recv(c_sock, buf, BUFFSIZE, 0) == -1) {
            perror("[S] Can't receive file name.\n");
            exit(1);
    }

     int request = 0; // 파일 이름 재전송위한 인수 

    while(1){

        printf("[S] Client request file name : %s\n", buf);

         //3. 파일을 갖고있는지 여부 메세지 전송
         rfp = fopen(buf,"r");
         if (rfp == NULL) {
             printf("[S] I don't have file\n");

             if (send(c_sock, "N",sizeof("N"), 0) == -1){
                 perror("[S] Can't send message that I have file.\n");
                 exit(1);
             }
             request = -1; // 3.1 파일 없다고 표시하고 메세지 보냄
         }else{ //3.2 파일이 있다는 메세지 보냄

            printf("[S] I have file : %s\n",buf);

            if (send(c_sock, "Y",sizeof("Y"), 0) == -1){
                perror("[S] Can't send message that I don't have file.\n");
                exit(1);
            }
            break;        
         }

         printf("[S] I'm waiting for client's reply!\n");

         if (request == -1) //5. 파일이 없어서 응답대기
         {
            if (recv(c_sock, buf, BUFFSIZE, 0) == -1) {
                perror("[S] Can't receive massege\n");
                exit(1);
            }
            if(buf[0] == 'N' && buf[1] == '\0') //5.1. 파일 안받음
            {
                printf("[S] 파일을 보내지 않고 종료합니다.\n");
                close(c_sock);
                close(s_sock);
                return 0;
            }else{ // 5.2. 파일이름 다시 입력받음
                rfp = fopen(buf,"r");
                if (rfp != NULL){
                    if (send(c_sock, "Y",sizeof("Y"), 0) == -1){
                        perror("[S] Can't send message that I don't have file.\n");
                        exit(1);
                    }
                    printf("[S] I have file : %s\n",buf);

                    break;
                }
            }
         }
             
    }
    // 파일에 딱 버퍼 사이즈 만큼 : Fsize = BUFFSIZE와 같음. 그러면 한번 더 루프 돌아서 0 을 send, Fsize =0;
    // 버퍼보다 약간 부족 : Fsize < BUFFSIZE일테고, 바로 break됨 (send 0 안함) Fsize = 0아님

    int Fsize;
    while(1){
        memset(buf,0,BUFFSIZE);
        Fsize = fread(buf, 1, BUFFSIZE, rfp);
        
        if(Fsize < BUFFSIZE){
            if (send(c_sock, buf, Fsize, 0) == -1) {
                perror("[S] Can't send file Contents\n");
                exit(1);
            }
            break;
        }
        if (send(c_sock, buf, BUFFSIZE, 0) == -1) {
                perror("[S] Can't send file Contents\n");
                exit(1);
            }
    }

    if(Fsize != 0)
    {
        if (send(c_sock, buf, 0, 0) == -1) {
                perror("[S] Can't send file Contents\n");
                exit(1);
            }
        
    }

    printf("[S] 파일 전송을 완료하였습니다.\n");

    fclose(rfp);
    close(c_sock);
    close(s_sock);

    return 0;
}