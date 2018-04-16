#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define NUMPROCESS 5 //총 생성해야할 프로세스 수

//table 구조체
typedef struct{
	int dest;
	int link;
	int metric;
}TABLE;

//table 저장 변수
TABLE origin[10];
TABLE compare[10];
TABLE*  point_origin= origin;
TABLE*  point_compare= compare;


int parent();
int child();

//process 생성 후 table 저장
int main(int argc, char **argv) {
	pid_t pids[NUMPROCESS];
	int run_p = 0;
	
	//numprocess 개수 만큼 process생성
	while(run_p < NUMPROCESS){
		//process array에 저장
		pids[run_p] = fork();
		
		//process 만들어지지 않은 경우 error 메세지 출력 후 함수 종료
		if(pids[run_p]<0){
			printf("error! Can't make proces\n");
			return -1;
		}
		
		//child인 경우 child함수 실행
		else if(pids[run_p] == 0){
			child();
			sleep(1);
			exit(0);
		}
		
		//parent 인 경우 parent 함수 실행
		else{
			parent();
		}
	run_p++;
	}
	return 0;
}

//parent 
int parent(){
	//자식프로세스가 돌아가는 동안 정지
	int status=0;
	pid_t w_pid;
	while((w_pid = wait(&status))>0);
	return 0;
}

//받아온 table 읽어서 저장하는 함수
void ReadNInsert(FILE* fp,TABLE** tablept){
	char s[100];
	char* token;
	while((fgets(s,100,fp))!=NULL){
		token=strtok(s," ");
		(*tablept)->dest = atoi(token) ; 
		(*tablept)->link = atoi(strtok(NULL," "));
		(*tablept)->metric = atoi(strtok(NULL," "));
		(*tablept)++;
	}
	fclose(fp);
}

//table 값 받아오는 함수
int init_table(){
		FILE* fp;
		fp = fopen("input.txt","r");
		ReadNInsert(fp, &point_origin);
		TABLE* immpt;
		for(immpt= origin; immpt < point_origin ; immpt++){
			printf("dest: %d, link: %d , metric: %d\n",immpt->dest,immpt->link,immpt->metric);
		}
		return 0;
}

//child - 파일로 받은 table 지역변수로 저장 후 출력
int child(){
	printf("c : child start! %d\n", getpid());
	init_table();
	return 0;
}
