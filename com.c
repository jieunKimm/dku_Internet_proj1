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
 	int dest;
    int link;
    int metric;
 }TABLE;

pthread_t tids[100];
int thds=0;
void * calculate(void*);
static void * handle(void *);
FILE* rfile;

//main
int main(int argc, char *argv[]){
	int srv_sock,cli_sock;
	int port_num, ret, node_num;
	struct sockaddr_in addr;
	int len,rc;
	int* p_num = &port_num;

//input값 잘못 들어온 경우
	if(argc != 4)	printf("excute form : ./com input.txt (port number) (node number)\n");
//input으로 들어온 대로 자신의 port번호 지정
	port_num = atoi(argv[2]);
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

	//쓰레드로 calculate실행 -> 다이스트라 알고리즘
	//쓰레드 열기
	pthread_create(&tids[thds], NULL, calculate,(void*) p_num);

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

	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}

	int h = 0;
	/* read from client host:port */
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	if (len == 0){
		ret = 0;
		pthread_exit(&ret);
	}
	fseek(rfile, 0, SEEK_END);    // 파일 포인터를 파일의 끝으로 이동시킴
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

//다이스트라 알고리즘
void* calculate(void* arg){
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
	
	port_num = *((int*)arg);
	char* title = malloc(sizeof(char)*20);
	sprintf(title, "receive%d.txt", port_num);
	FILE* revalue = fopen(title, "w");

	sleep(5);
	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return 0;
	}
	cur_net = "127.000.000.001";
	if(port_num==3333)
		ser_port = 3434;
	else
		ser_port = 3333;
	// addr binding, and connect
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (ser_port);
	inet_pton(AF_INET, cur_net, &addr.sin_addr);
				ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return 0;
	}
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
		fflush(NULL);
}
