#include<stdio.h>
#include <string.h>
#include <stdlib.h>

#define BK 10 //basket num
#define SL 3  //dimension


int  hashtable_origin[BK][SL];
int hashtable_compare[BK][SL];
 
int hash(int key)
{
	return key /1000;
}


void AddTable(char* s,int* table)

{
	int bucket;
	char* token;
	int dest=0;
	int link = 0;
	int metric =0;
	
	token=strtok(s," ");
	dest = atoi(token) ; 
	link = atoi(strtok(NULL," "));
	metric = atoi(strtok(NULL," "));
	bucket = hash(dest);
	if(table[bucket][0] == 0){ 
		table[bucket][0] = dest;
		table [bucket][1] = link;
		table [bucket][2] = metric;
	}
	else
		printf("already filled"); 
		
}

  

int FindKey(int key)

{
    int bucket;
    bucket = hash(key);
    return (hashtable[bucket][0] == key);

}


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
	fclose(fp);

}
int main(){
	FILE* fp;
	fp=fopen("input2.txt","r");
	ReadNInsert(fp,hashtable_origin);




}


