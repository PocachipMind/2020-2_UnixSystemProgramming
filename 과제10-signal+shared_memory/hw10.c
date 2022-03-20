#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>

#define BUFSIZE (80)

void handler(int signo)
{
    psignal(signo, "Received signal:");
}

int main(void)
{
    pid_t pid;
    int wstatus;
    struct sigaction act;
    key_t key;
    int shmid, size;
    char buf[BUFSIZE];
    void *shmaddr;
    sigset_t set_to_wait_and_block;
    FILE *fp;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_flags = 0;
    act.sa_handler = handler;
    sigaction(SIGUSR1, &act, NULL); //SIGUSR1 handler 설정

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR2);
    sigaction(SIGUSR2, &act, NULL); //SIGUSR2 handler 설정


    pid = fork();

    if (pid < 0) //에러
    {
        perror("Fork Failed");
        return 1;

    } else if (pid == 0) //자식 프로세스
    {

        sigfillset(&set_to_wait_and_block);
        sigdelset(&set_to_wait_and_block, SIGUSR1); //SIGUSR1만 제외된 집합 설정

        sigprocmask(SIG_BLOCK, &set_to_wait_and_block, NULL); // 시그널 집합에 대해 블록(SIGUSR1빼고 블록)

        sigsuspend(&set_to_wait_and_block); // 집합에서 제외된 시그널(SIGUSR1) 대기

        
        key = ftok("shmfile",1); //shmfile을 이용해 키 생성. 파일이 존재해야함
        size = sysconf(_SC_PAGESIZE); //페이지 사이즈 = 4KB
        shmid = shmget(key, size, IPC_CREAT|0666); //공유 메모리 접근 권한 설정

        shmaddr = shmat(shmid,NULL,0); //할당 받은 공유 메모리 공간의 주소를 얻음

        memcpy(buf, shmaddr, BUFSIZE); //현재 메모리 공간의 처음 80B 내용 확인.
        printf("Child Process: %s\n",buf); //0으로 초기화 하였으니 아무런 내용 없음

        fp = fopen("shmfile","r"); //파일오픈
        if (fp == NULL)
        {
            perror("File Open");
            exit(1);
        }
        fgets(buf,BUFSIZE,fp);//buf에 파일의 문자열 가져옴
        fclose(fp);
        memcpy(shmaddr, buf, BUFSIZE);
        
        shmdt(shmaddr); //공유 메모리 공간 연결 해제

        kill(getppid(),SIGUSR2);

        return 0;

    }else //부모 프로세스
    {
        // 키로 사용할 파일은 임의의 문자열로 작성 구현
        printf("키로 사용할 파일의 임의 문자열을 입력해 주세요 :\n");
        scanf("%[^\n]s",buf);
        fp = fopen("shmfile","w");
        if (fp == NULL)
        {
            perror("File Open");
            exit(1);
        }
        fprintf(fp,"%s",buf);
        fclose(fp);

        key = ftok("shmfile",1); //shmfile을 이용해 키 생성. 파일이 존재해야함, 앞에서 생성.
        size = sysconf(_SC_PAGESIZE); //페이지 사이즈 = 4KB
        shmid = shmget(key, size, IPC_CREAT|0666); //공유 메모리 접근 권한 설정

        shmaddr = shmat(shmid,NULL,0); //할당 받은 공유 메모리 공간의 주소를 얻음

        memset(shmaddr,0,size); //공유 메모리 공간 전체 0으로 초기화

        memcpy(buf, shmaddr, BUFSIZE); //해당 공간의 처음 80B 만큼의 데이터 복사하여 출력
        printf("Initialized: %s\n", buf); //0으로 초기화 하였으니 아무내용 없음

        kill(pid,SIGUSR1); // child process에게 시그널 보냄

        sigfillset(&set_to_wait_and_block);
        sigdelset(&set_to_wait_and_block, SIGUSR2); //SIGUSR2만 제외된 집합 설정

        sigprocmask(SIG_BLOCK, &set_to_wait_and_block, NULL); // 시그널 집합에 대해 블록(SIGUSR2빼고 블록)

        sigsuspend(&set_to_wait_and_block); // 집합에서 제외된 시그널(SIGUSR2) 대기

        //sigprocmask(SIG_UNBLOCK,&set_to_wait_and_block, NULL); //블록해제


        memcpy(buf, shmaddr, BUFSIZE); //해당 공간의 변경된 내용 확인
        printf("Changed: %s\n", buf);

        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID,NULL); //공유 메모리 공간 할당 해제
        


        wait(&wstatus);
        printf("Parent: Child says %d\n",wstatus);

        return 0;
    }

    
}