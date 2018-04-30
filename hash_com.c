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

#define SL 3 // dimension 
#define BK 10 //basket num

//structure for table node
typedef struct{
 	int flag;
	int dest;
	int link;
        int metric;
 }TABLE;


FILE* fp_org;
FILE* fp_cmp;
TABLE* findkey(int key);

//Variable of tables
TABLE***  hashtable_origin; 
TABLE*** hashtable_compare;

//variable about lowdestination
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
int hash(int key);

void AddTable(FILE* fp,TABLE*** table) //Read File and insert to table 
{
	char string[100];
	int bucket;
	char* token;
	int i;
        while((fgets(string,100,fp))!=NULL) //get line of file
        { 
	TABLE* imm=(TABLE*)malloc(sizeof(TABLE)); // make imm node
	printf("string :%s \n", string);
        token=strtok(string," ");
        printf("tokenwell\n");
        imm->dest = atoi(token) ;
        imm->link = atoi(strtok(NULL," "));	
        imm->metric = atoi(strtok(NULL," "));
	imm->flag=0;
        bucket = hash(imm->dest);     //dest is key, so find bucket by hash function
	for(i=0;table[bucket][i]!=NULL;i++); // find empty space
	table[bucket][i]= imm; 
 	}
}

void addtotable(int dest,TABLE* table) //insert node info to table
{
	int i= 0;
	int bucket = hash(dest);
	for(i=0;hashtable_origin[bucket][i]!=NULL;i++);
	hashtable_origin[bucket][i] =table;

}
void calculate(TABLE*** origin, TABLE*** compare) // compare two tables and updated 
{
	int i=0;
	int j=0; 
	int check = 0;
	int dest;
	TABLE* orgpt;
	TABLE* imm;
	int* path_pointer = path ;
	for( i=0;i<BK ;i++)
	{
		for(j=0 ; j<SL ; j ++)    // for check all items in hashtable_compare
		{	
			check = 0;
			//printf("i:%d j:%d",i,j);
			if (compare[i][j] ==NULL ) continue; //if there is no element in compare table, skip it
			for(path_pointer=path;*path_pointer !=0;path_pointer++) // check whether this node is in path or not
			{
				if( * path_pointer ==compare[i][j]->dest)
				{	check =1;
					break;		
				}	
			}
			if(check == 1) // if it is in path,then skip
				continue;
			else
			{
				dest = compare[i][j]->dest;
				orgpt= findkey(dest);
				if(orgpt==NULL) // if this dest info is not in origin
				{
					
				        imm=(TABLE*)malloc(sizeof(TABLE));
					imm->dest = compare[i][j]-> dest;
					imm->link = lowdest;
					imm->metric = compare[i][j]->metric + lowmetric;
					addtotable(dest,imm); //add to table
					continue;
					
				}
				else{ // if this data info in in origin
					int comparemetric=0;
					comparemetric = compare[i][j]->metric + lowmetric;
					if( orgpt->metric > comparemetric) //compare metric and update when the origin_metric is lower 
					{
						orgpt->link  = lowdest;
						orgpt -> metric = comparemetric;
						printf("orgptlink %d,orgpt metric%d",orgpt->link,orgpt->metric);
						printf("metric is updated \n");
					}
				}

			
			}
		}
	}
}
	


  
void printTable(char* s, TABLE*** table)
{
	int i = 0;
	int j = 0 ;
	printf("----------------------------%s_table---------------------------------\n",s);
	for(i= 0 ;i <BK ;i++)
	{	//printf("check node_num:%d\n",i);
		for(j=0 ; j < SL ; j++)
			if(table[i][j] !=  NULL) 
				printf("flag:%d , dest: %d, link: %d , metric: %d\n",table[i][j]->flag,table[i][j]->dest,table[i][j]->link,table[i][j]->metric);
	}	
	 printf("---------------------------------------------------------------------------\n");

}

TABLE* findkey(int key) // find where the data stores.

{
    printf("key: %d \n",key);
    int bucket;
    bucket = hash(key);
    int j=0;
    if( hashtable_origin[bucket][0]==NULL) //if there is no data in array
	 	return NULL; //return
    for(;j<SL;j++){

	if(hashtable_origin[bucket][j]==NULL) // if there is no data which matches we want
	{
		printf("return null\n");
		return NULL; //return
	}
        else if(hashtable_origin[bucket][j]->dest == key) //when there is a data we want 
                return hashtable_origin[bucket][j];

    }
    printf("already full error \n");
    return NULL; 
}

void findShortest(void) // find shortest destination among the origin table data 
{
	int i;
	int j;
	int low_index;
	int low_column;
	lowdest =0;
	lowmetric = 0;
	printf("enter findshortest function \n");
	printf("low metric : %d ",lowmetric);
	for(i=0 ; i < BK ; i++)
	{
		for(j=0 ; j< SL ; j++)
		{	
	
			//printf("%p\n",immpt);
			if(hashtable_origin[i][j] == NULL){
				//printf("no information");      //no information
				continue;
			}
			if(hashtable_origin[i][j]->flag == 1)  // already checked 
				continue;
			if(lowmetric == 0)  //first input
			{
				//printf("meet first data\n");
				lowmetric = hashtable_origin[i][j]-> metric;
				lowdest = hashtable_origin[i][j]->dest; 
				//printf("current lowmetric ; %d ,lowdest : %d ",lowmetric ,lowdest );
				low_index = i;
				low_column=j;
				continue;	
			}
			if(lowmetric > (hashtable_origin[i][j]->metric)) //if there is a lower metric, then update
			{
				lowmetric = hashtable_origin[i][j]->metric;
				lowdest = hashtable_origin[i][j]->dest;
                                low_index = i;
                                low_column=j;

			}			
		
		}
	}
	if(lowdest ==0) // all items are already checked 
		return ;
	hashtable_origin[low_index][low_column]->flag = 1 ; 
}

int hash(int key)
{
	return key /1000;
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
	TABLE*** imm;
	imm = (TABLE***) malloc(sizeof(TABLE **) * BK);
	int i,j;
	for (i=0 ; i<BK ; i++)       //allocating Table with dynamic allocation
	{ 
		imm[i] = (TABLE **)malloc(sizeof(TABLE) * SL); 
	}
	 for (i=0; i<BK; i++)
         {
                        for(j=0;j<SL;j++)
                                imm[i][j] = NULL;
         }
 
	hashtable_origin = imm;
	rfile = fopen((char *) argv[1], "r");
	if(rfile == NULL){
		printf("there are no such file\n");
		exit(0);
	}
	else printf("read input file\n");
	send_pointer = rfile;
	AddTable(rfile,hashtable_origin);
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
	TABLE*** imm;
	char* title=malloc(sizeof(char)* 20 ); // variable for storing file name 
	printf("nodeNum: %d\n",nodeNum);
	printTable("origin",hashtable_origin);
	findShortest();
	printf("low dest : %d \n",lowdest);
 	*p_path = lowdest;
	 p_path++;
	printf("nodeNum:%d\n",nodeNum)	;
	int i;
	int j;
	int k= 0 ;
	for(k=0;k <=(nodeNum-2);k++)
	{
		imm = (TABLE***) malloc(sizeof(TABLE **) * BK);
        	for (i=0 ; i<BK ; i++)
        	{
       			imm[i] = (TABLE **)malloc(sizeof(TABLE) * SL);
        	}
		for (i=0; i<BK; i++)
		{
			for(j=0;j<SL;j++)
  				imm[i][j] = NULL;
		}
		hashtable_compare = imm; 
		title=client(lowdest);
		fp_cmp = fopen(title,"r");
		printf("connect\n");
		AddTable(fp_cmp,hashtable_compare);
		printTable("origin",hashtable_origin);
		printTable("compare",hashtable_compare);
		calculate(hashtable_origin,hashtable_compare);
        	printTable("updated",hashtable_origin);
       		findShortest();
       		printf("low dest : %d \n",lowdest);
        	*p_path = lowdest;
		p_path++;
	}
	printf("dijstra path:");

        for(int* imm=path;*imm!=0;imm++) //dijkstra path print
        	printf("%d-",*imm);
	printf("\n");
	printTable("Final",hashtable_origin);
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
