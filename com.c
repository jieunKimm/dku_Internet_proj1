/*
de들이 실행해야하는 코드 정리*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

typedef struct{
 	int flag;
	int dest;
	int link;
        int metric;
 }TABLE;
FILE* fp_org;
FILE* fp_cmp;
TABLE origin[10]={0};
TABLE compare[10]={0};
TABLE*  point_origin= origin;
TABLE*  point_compare= compare;
int lowmetric=0;
int lowdest=0;
TABLE* lowpt;
int path[10]={0};
int* p_path = path;
pthread_t tids[100];
int thds=0;
char * client(int portnum);
void * dijkstra(void*);
static void * handle(void *);
FILE* rfile;
FILE* send_pointer;
void ReadNInsert(FILE* fp,TABLE* tablept){
	char s[100];
	char* token;
	while((fgets(s,100,fp))!=NULL)
	{
		token=strtok(s," ");
		(tablept)->dest = atoi(token) ; 
		(tablept)->link = atoi(strtok(NULL," "));
		(tablept)->metric = atoi(strtok(NULL," "));
		(tablept)++;
	}
	//fclose(fp);

}

void calculate(TABLE* origin,TABLE* compare,int* path)
{
	int* path_p = path;
	int check=0;
	int comparemetric = 0;
	TABLE* origin_p = origin;
	for(; compare->dest!=0 ; compare++,origin=origin_p,path=path_p,check=0)
	{
		printf("compare dest:%d\n",compare->dest);
		for(;*path!= 0;path++)
		{	
			if(compare->dest==*path)
			{	
				printf("path value is :%d\n",*path);
				printf("check become 1\n ");
				check=1; //already this node is checked
				break;
			}
		}
		if(check==1)
			continue;
		else
		{
			for( ;origin->dest!= compare->dest; origin++)
			{	
				if(origin->dest==0)
					break;
			}
			if(origin->dest==0)
			{
				origin->dest = compare->dest;
				origin->link = lowdest;
				origin->metric= compare->metric+ lowmetric;
				continue;
			}
			else
			{
				comparemetric = compare->metric + lowmetric;
				printf("cmoparemetric:%d, origin metric:%d ",comparemetric,origin->metric);	
				if(origin->metric > comparemetric)
					{
						origin->link= lowdest;
						origin->metric= comparemetric;
						printf("metric is updated\n");
					}
			}	
	
				

		}
			
	}


}

void printTable(char* s,TABLE* table)
{
	TABLE* immpt;
	 printf("----------------------------%s_table---------------------------------\n",s);
	 for(immpt=table; immpt->dest != 0 ; immpt++){
                printf("flag:%d , dest: %d, link: %d , metric: %d\n",immpt->flag,immpt->dest,immpt->link,immpt->metric);
        }
        printf("---------------------------------------------------------------------------\n");

}

void findShortest()
{
	TABLE* immpt;
        for (lowmetric=0,lowdest=0,immpt = origin; immpt ->dest !=0 ; immpt++)   // until end of file
        {
                if(immpt->flag ==1)
                        continue;
                if (lowmetric == 0)             // the first time fo comparision --> initialize low dest and metric
                {
                        lowpt=immpt;
                        lowmetric = immpt->metric;
                        lowdest = immpt->dest;
                }
                 if (lowmetric > immpt->metric)  // if there is dest which has lower metric then change value
                {
                        lowpt=immpt;
                        lowmetric = immpt->metric;
                        lowdest = immpt->dest;
                }
        }
	lowpt->flag = 1;
}
//main
int main(int argc, char *argv[]){
	int srv_sock,cli_sock;
	int port_num, ret, node_num;
	struct sockaddr_in addr;
	int len,rc;
	int* p_num = &node_num;	
//input값 잘못 들어온 경우
	if(argc != 4)	printf("excute form : ./com input.txt (port number) (node number)\n");
//input으로 들어온 대로 자신의 port번호 지정
	port_num = atoi(argv[2]);
	*p_path = port_num;
	p_path ++ ;
//input으로 들어온 대로 노드 개수 저장
	node_num = atoi(argv[3]);
//input파일을 받아서 자신의 table정보 채우기
	//file 읽기
	rfile = fopen((char *) argv[1], "r");
	if(rfile == NULL){
		printf("there are no such file\n");
		exit(0);
	}
	else printf("read input file\n");
	send_pointer = rfile;
	ReadNInsert(rfile, point_origin);
	//쓰레드로 calculate실행 -> 다이스트라 알고리즘
	//쓰레드 열기
	pthread_create(&tids[thds], NULL, dijkstra,(void *)p_num);

//server로써의 역할
	//socket열기
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

//addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr.sin_port = htons (port_num); // using port num

	ret = bind(srv_sock, (struct sockaddr *)&addr, sizeof(addr));	
	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	//listen
	for (;;){
	// Listen part
		ret = listen(srv_sock, 0);
		if (ret == -1) {
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			return 0;	
		}
	
	//요청오면 accept -> cli저장
		cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
		if (cli_sock == -1) {
			perror("cli_sock connect ACCEPT fail");
			close(srv_sock);
		}
		thds++;
	
	// cli handler
	//thread를 통해 cli와 통신 -> thread 실행 함수 -> handle
		pthread_create(&tids[thds], NULL, handle, &cli_sock);

	//만약모든 node에게 정보 주었다면 
		if(thds == node_num-1){ 
			close(srv_sock);
			break;
		}
	}// end for

//모든 thread가 끝날때까지 기다리기
	for(int i=0;i<=thds;i++) rc = pthread_join(tids[i], (void**)&ret);
	return 0;
}

//handle
//the process send data with thread that execute handle
static void * handle(void * arg){
	printf("handle thread open\n");
	int cli_sockfd = *(int *)arg;
	int len,size;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1024);
	char *send_buffer;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int port_num =*((int*)arg);
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len, 
		hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
		NI_NUMERICHOST | NI_NUMERICSERV); 

    fread(recv_buffer, 100, 1, send_pointer);  //파일 크기만큼 값을 읽음
	printf("recv_buffer : %s", recv_buffer);
	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	if (len == 0){
		ret = 0;
		pthread_exit(&ret);
	}

	if (fseeko(send_pointer, 0, SEEK_END)!= 0)    // 파일 포인터를 파일의 끝으로 이동시킴 
		printf("ok fseek not complete\n");
	size = ftell(rfile);          // 파일 포인터의 현재 위치를 얻음
    send_buffer = malloc(size + 1);    // 파일 크기 + 1바이트(문자열 마지막의 NULL)만큼 동적 메모리 할당
    memset(send_buffer, 0, size + 1);  // 파일 크기 + 1바이트만큼 메모리를 0으로 초기화
    fseek(rfile, 0, SEEK_SET);          // 파일 포인터를 파일의 처음으로 이동시킴
    fread(send_buffer, size, 1, rfile);  //파일 크기만큼 값을 읽음
		
	len = strlen(send_buffer);
	printf("send buffer %s\n ",send_buffer);
	send(cli_sockfd,send_buffer,len,0);
	close(cli_sockfd);
	free(recv_buffer);
	free(send_buffer);
	
	ret = 0;
	pthread_exit(&ret);
}

void* dijkstra(void* arg){
	int nodeNum = *((int *)arg);
	char* title=malloc(sizeof(char)* 20 );
	printf("nodeNum: %d\n",nodeNum);
	printTable("initial",origin);
	findShortest();
	printf("low dest : %d \n",lowdest);
 	*p_path = lowdest;
	 p_path++;
	printf("nodeNum:%d\n",nodeNum)	;
	int i= 0 ;
	for(i=0;i<=(nodeNum-2);i++)
	{
		title=client(lowdest);
		fp_cmp = fopen(title,"r");
		printf("connect\n");
		ReadNInsert(fp_cmp,point_compare);
		printTable("origin",origin);
		printTable("compare",compare);
		calculate(origin,compare,path);
        	printTable("updated",origin);
        	memset(&compare,0,sizeof(compare));
       		findShortest();
       		printf("low dest : %d \n",lowdest);
        	*p_path = lowdest;
		p_path++;
	}
	printf("dijstra path:");
        for(int* imm=path;*imm!=0;imm++)
        	printf("%d-",*imm);
	printf("\n");
	printTable("Final",origin);
	free(title);
	return 0;
}
//다이스트라 알고리즘
char* client(int arg){
	int fd_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;
	size_t getline_len;
	char* cur_net;
	char *buffer;
	char r_buffer[1024];
	int flag = 0;
	int ser_port = 0;
	printf("enter to client function %d\n", arg);	
	ser_port = arg;
	char* title = malloc(sizeof(char)*20);
	sprintf(title, "receive%d.txt", getpid());
	FILE* revalue = fopen(title, "w");

	sleep(5);
	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return 0;
	}
	cur_net = "127.000.000.001";
	// addr binding, and connect
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (ser_port);
	inet_pton(AF_INET, cur_net, &addr.sin_addr);
	printf("before connect\n");
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return 0;
	}
	printf("after binding\n");
	buffer = "want connect";
	len = strlen(buffer);
	send(fd_sock, buffer, len, 0);
	while(1){	
		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);	
		if(len >0) break;
	}
		printf("server says $ %s,%d\n", r_buffer,len);
		fwrite(r_buffer, 1, len,revalue);
		close(fd_sock);
	//	fflush(NULL);
	fclose(revalue);
	return title;
}
