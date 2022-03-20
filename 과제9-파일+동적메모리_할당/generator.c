#include <stdio.h> // 9.File High-level IO 강의자료의 예제 7번(40 page)을 참고하여 프로그램을 작성하였습니다.
#include <stdlib.h>
#include <time.h> // seed를 변화시키기 위한 include
#include <string.h> // 문자열 복사를 위한 include

char* names[10] = { "Alice", "Bob", "Chris", "Dod", "Evan", "Fint", "Gregg", "Brendan", "Roy", "Susan"};

struct pscore
{
    int num; //학번
    char nme[10]; //이름
    float sc1; // 점수1
    float sc2; // 점수2
    float sum; // 점수합계
};

int main(int argc, char* argv[])
{
    FILE *fp;

    srand(time(NULL)); //time 함수를 통해 매번 다른 seed값으로 변경
    int datanum = (rand() % 1000) + 1; //1 ~ 1000 난수 생성 > 0이아닌 1부터 시작 : 문제에서 임의의 학생 데이터 "생성하여" 라고 나와있음.
    //datanum은 데이터가 몇개 생성될지 갯수.
    
    if ((fp = fopen("unix.bin","w")) == NULL) // unix.bin으로 열고 실패시 예외처리
    {
        perror("Open");
        exit(1);
    }

    int i;
    struct pscore data;

    for(i=0; i<datanum; i++)
    {
        data.num = ( rand() % 10000 ) + ( rand() % 100 )*10000 + 201000000; // 201****** 인 무작위 학번 생성(rand가 최대 32767까지 반환하므로)
        strcpy(data.nme, names[rand() % 10]); // names 배열에서 무작위 하나 선택 후 복사
        data.sc1 = ( rand() / (float)RAND_MAX )*100.0f; // 최대 100점인 무작위 실수 생성
        data.sc2 = ( rand() / (float)RAND_MAX )*100.0f; // 최대 100점인 무작위 실수 생성
        data.sum = data.sc1 + data.sc2; //sc1,sc2 더함

        fwrite(&data,sizeof(struct pscore),1,fp);
        
    }

    printf("unix.bin파일로 나간 구조체 갯수: %d\n", datanum);
    
    fclose(fp);

    return 0;
    
}