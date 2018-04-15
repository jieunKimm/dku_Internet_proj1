#include<stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
	int dest;
	int link;
	int metric;
}TABLE;

TABLE origin[10];
TABLE compare[10];
TABLE*  point_origin= origin;
TABLE*  point_compare= compare;

void ReadNInsert(FILE* fp,TABLE** tablept){
	char s[100];
	char* token;
	while((fgets(s,100,fp))!=NULL)
	{
		token=strtok(s," ");
		(*tablept)->dest = atoi(token) ; 
		(*tablept)->link = atoi(strtok(NULL," "));
		(*tablept)->metric = atoi(strtok(NULL," "));
		(*tablept)++;
	}
	fclose(fp);


}
int main(void)
{
	FILE* fp;
	fp = fopen("input.txt","r");
	ReadNInsert(fp, &point_origin);
	TABLE* immpt;
	for(immpt= origin; immpt < point_origin ; immpt++){
		printf("dest: %d, link: %d , metric: %d\n",immpt->dest,immpt->link,immpt->metric);
	}
	return 0;
}
