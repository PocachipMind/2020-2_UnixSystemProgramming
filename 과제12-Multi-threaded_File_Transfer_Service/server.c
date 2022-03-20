#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799
#define MAX_THREADS (10)

pthread_t tid[MAX_THREADS];

struct twovalue{
    int inum;
    int sock;
};

void* sending (void *arg) {
    int i, *ret;
    struct twovalue indent = *((struct twovalue *)arg);
    char buf[BUFFSIZE]; // 클라이언트와 전송할때 쓸 버퍼
    char division[80]; 
    ssize_t check; //QnA 내용처럼 send와 recv 반환값 검사용 변수
    FILE *rfp;


    ret = (int *)malloc(sizeof(int)); //스레드 종료때 반환값위해
    
    srand((unsigned int)indent.inum);
    sleep(rand()%5);

    for(i=0; i<indent.inum; i++) division[i]='\t';

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec); //시작시간 출력

    for(i=0; i<3; i++) {
        printf("%s%d...\n", division, i);
        sleep(1);
    }

    // file name 받기
    check = recv(indent.sock,buf,BUFFSIZE, 0);
    if ((check == -1) || (check == 0)) {
        printf("%sCan't receive message\n", division);
        close(indent.sock);
        *ret = -1;
        t = time(NULL);
        tm = *localtime(&t);
        printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
        pthread_exit(ret);
    }

    rfp = fopen(buf,"rb");
    if (rfp == NULL) { //파일이 없음
        printf("%sNo File\n", division);
        check = send(indent.sock,"N",sizeof("N"), 0); //파일 없다고 메세지보냄
        if ( (check == -1) || (check == 0) ) {
            printf("%sCan't send message\n", division);
        }
        close(indent.sock);
        *ret = 0;
        t = time(NULL);
        tm = *localtime(&t);
        printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
        pthread_exit(ret);
    }

    check = send(indent.sock,"Y",sizeof("Y"), 0); //파일이 있음, 있다고 메세지보냄
    if ((check == -1) || (check == 0)) {
        printf("%sCan't send message\n", division);
        close(indent.sock);
        *ret = -1;
        t = time(NULL);
        tm = *localtime(&t);
        printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
        pthread_exit(ret);
    }

    //파일 전송
    while(1){ 
        memset(buf,0,BUFFSIZE);
        indent.inum = fread(buf, 1, BUFFSIZE, rfp);
        
        if(indent.inum < BUFFSIZE){
            
            if (send(indent.sock, buf, indent.inum, 0) == -1) {
                printf("%sCan't send\n", division);
                close(indent.sock);
                *ret = -1;
                t = time(NULL);
                tm = *localtime(&t);
                printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
                pthread_exit(ret);
            }
            break;
        }
        if (send(indent.sock, buf, BUFFSIZE, 0) == -1) {
                printf("%sCan't send\n", division);
                close(indent.sock);
                *ret = -1;
                t = time(NULL);
                tm = *localtime(&t);
                printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
                pthread_exit(ret);
            }
    }

    fclose(rfp);
    close(indent.sock);

    printf("%sSent\n", division);
    *ret = 1;
    t = time(NULL);
    tm = *localtime(&t);
    printf("%s%d:%d\n",division,tm.tm_min, tm.tm_sec);
    pthread_exit(ret);
}

int main(void) {
    int *status, tcounts = 0; // 스레드 반환값, 스레드 생성갯수
    struct twovalue args[MAX_THREADS]; // 인자 전달 위해
    int i, s_sock, c_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};

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

    printf("#00\t#01\t#02\t#03\t#04\t#05\t#06\t#07\t#08\t#09\n"); // 10개의 쓰레드 명시해주기 위해

    listen(s_sock,10); //동시 접속을 허용할 최대 클라이언트 수 10개
    c_addr_size = sizeof(struct sockaddr);

    for(i=0; i<10; i++, tcounts++) {
        c_sock = accept(s_sock, (struct sockaddr *) &client_addr, &c_addr_size);
        if (c_sock == -1) {
            perror("[S] Can't accept a connection"); //accept 예외처리
            exit(1);
        }

        args[i].inum = i;
        args[i].sock = c_sock; //연결된 클라이언트 소켓 전달
        if ( pthread_create(&tid[i], NULL, sending, &args[i]) != 0 ) {
            perror("Failed to create thread");
            goto exit;
        }

    }
    
exit:
    // 반환값 출력
    for(i=0; i<tcounts; i++) {
        pthread_join(tid[i], (void **) &status);
        printf("Thread no.%d ends: %d\n", i, *status);
    }
    close(s_sock);

    return 0;
}