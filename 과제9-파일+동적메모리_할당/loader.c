#include <stdio.h> // 예전에 작성한 배열기반 출력프로그램에서 연결리스트로 수정하겠습니다.
#include <stdlib.h> //10. Dynamic Memory Allocation 강의자료의 예제 2번(34 page)을 참고하여 프로그램을 작성하였습니다.

struct pscore
{
    int num; //학번
    char nme[10]; //이름
    float sc1; // 점수1
    float sc2; // 점수2
    float sum; // 점수합계
};

typedef struct selfref
{
    struct pscore content;
    struct selfref *next; 
} node;
// 노드라는 새로운 구조체를만들고, 노드 안에 노드를 가르키는 포인터와 pscore를 갖도록 하였음.

int main(int argc, char* argv[])
{
    FILE *fp;
    node *head, *cur, *data, *prev;

    if ((fp = fopen("unix.bin","r")) == NULL) // 열린지 확인
    {
        perror("Open");
        exit(1);
    }

    cur = (node*) malloc(sizeof(node));
    head = cur; //헤드를 처음 생성한 node object로 지정
    prev = cur; //맨 마지막에 불필요하게 할당된 공간을 해제하고 이전 노드의 link를 null로 업데이트 하기위함
    cur->next = NULL; //다음 항목은 아직 없으므로 NULL로 초기화

    while( fread(&(cur->content),sizeof(struct pscore), 1, fp) == 1 )
    {
        cur->next = (node*) malloc(sizeof(node)); //다음 정보를 저장할 노드 생성하고, 이전 노드가 새로운 노드를 가리키게 함
        prev = cur; //맨 마지막에 불필요하게 할당된 공간을 해제하고, 이전 노드의 link를 NULL로 업데이트 하기위함
        cur = cur->next; //cur를 이용해 새로운 데이터를 읽어야 하므로, cur포인터를 새 노드를 가리키도록 업데이트
        cur->next = NULL; //다음 항목은 아직 없으므로 NULL로 초기화
    }

    prev->next = NULL; //마지막에 할당된 공간은 해제하였으므로 이전노드의 next = NULL
    free(cur); //마지막 할당된 공간은 불필요하므로 해제

    if( head == cur )
    {
        head = NULL; //만약 cur와 head가 같다면 어떤 데이터도 로드하지 못한 상태. head도 NULL로 지정
    }

    // 정수 입력받고 데이터 출력

    int printnum = -1; //정수 입력받을 변수

    while(1)
    {    
        fprintf(stdout, "출력하실 데이터의 노드 번호를 입력해주세요.\n번호:");
        fscanf(stdin, "%d", &printnum); //정수 입력
        if( (printnum<=0) || (printnum>1000) ) // 입력정수가 0~1000인지 확인
        {
            fprintf(stdout, "노드의 번호는 1000이하의 양의 정수여야합니다! 다시 입력해주세요.\n");
            continue; // 올바른 정수를 입력할때 까지 계속 입력 받음.
        }else //0~1000사이 입력받음
        {
            cur = head;

            int i;

            for(i=0; i<printnum; i++)
            {
                prev = cur;
                if(prev == NULL) // 마지막 노드에 도달
                {
                    fprintf(stdout, "%d 번 노드는 갖고 있지 않습니다!\n",printnum); //너무 정수가 큼( 갖고있는 노드보다 높은 정수 )
                    fprintf(stdout, "최대 %d 번까지 노드를 갖고 있습니다. %d 이하의 정수를 다시 입력해주세요.\n",i,i);
                    break; // 정수 다시 입력받음
                }
                cur = cur->next;
            }

            if( i == printnum ) // 마지막 노드에 도달안하고 for문 탈출했는지 점검(즉 printnum번 노드가 갖고있는 노드인지 점검))
            {
                fprintf(stdout, "%d 번 노드의 데이터 :\n", printnum);
                fprintf(stdout, "학번: %d 이름: %s 점수1: %.2f 점수2: %.2f 합계: %.2f\n", prev->content.num, prev->content.nme,prev->content.sc1,prev->content.sc2,prev->content.sum);
                break; // 노드 데이터 출력했으니 while문 탈출
            }

        }
        
    }

    cur = head;

    while(cur != NULL) //동적 할당 해제 루프
    {
        prev = cur; //이전 노드 주소 복제
        cur = cur->next; //cur를 다음 노드를 가리키도록 업데이트
        free(prev); //이전 노드 할당 해제
    }

    fclose(fp);

    return 0;

}