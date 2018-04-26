#include<stdio.h>
#include <string.h>
#include <stdlib.h>

#define SL 3 // dimension 
#define BK 10 //basket num



typedef struct
{
	int flag;
	int dest;
	int link;
	int metric;
}TABLE;

int hash(int key)
{
	return key /1000;
}
TABLE* findkey(int key);
TABLE***  hashtable_origin;
TABLE*** hashtable_compare;
int path[10]={0};
int* p_path = path ;
int lowmetric=0 ;
int lowdest = 0;
int* lowpt ; 
FILE* fp_cmp;
FILE* fp_org;
void AddTable(FILE* fp,TABLE*** table)
{
	char string[100];
	int bucket;
	char* token;
	int i;
        while((fgets(string,100,fp))!=NULL)
        { 
	TABLE* imm=(TABLE*)malloc(sizeof(TABLE));
	printf("string :%s \n", string);
        token=strtok(string," ");
        printf("tokenwell\n");
        imm->dest = atoi(token) ;
        imm->link = atoi(strtok(NULL," "));	
        imm->metric = atoi(strtok(NULL," "));
	imm->flag=0;
        bucket = hash(imm->dest);
	for(i=0;table[bucket][i]!=NULL;i++);
	table[bucket][i]= imm;
 	}
}

void addtotable(int dest,TABLE* table)
{
	int i= 0;
	int bucket = hash(dest);
	for(i=0;hashtable_origin[bucket][i]!=NULL;i++);
	hashtable_origin[bucket][i] =table;

}
void calculate(TABLE*** origin, TABLE*** compare)
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
		for(j=0 ; j<SL ; j ++)
		{	
			check = 0;
			//printf("i:%d j:%d",i,j);
			if (compare[i][j] ==NULL ) continue; 
			for(path_pointer=path;*path_pointer !=0;path_pointer++)
			{
				if( * path_pointer ==compare[i][j]->dest)
				{	check =1;
					break;		
				}	
			}
			if(check == 1)
				continue;
			else
			{
				dest = compare[i][j]->dest;
				orgpt= findkey(dest);
				if(orgpt==NULL)
				{
					
				        imm=(TABLE*)malloc(sizeof(TABLE));
					imm->dest = compare[i][j]-> dest;
					imm->link = lowdest;
					imm->metric = compare[i][j]->metric + lowmetric;
					addtotable(dest,imm);
					continue;
					
				}
				else{
					int comparemetric=0;
					comparemetric = compare[i][j]->metric + lowmetric;
					if( orgpt->metric > comparemetric)
					{
						orgpt->link  = lowdest;
						orgpt -> metric = comparemetric;
						printf("orgptlinke%d,orgptmetric%d",orgpt->link,orgpt->metric);
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

TABLE* findkey(int key)

{
    int bucket;
    bucket = hash(key);
    int j=0;
    for(;j<SL;j++)
	if(hashtable_origin[bucket][j]->dest == key)
   		 return hashtable_origin[bucket][j];
    return NULL;
}

void findShortest(void)
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
			if(lowmetric > (hashtable_origin[i][j]->metric))
			{
				lowmetric = hashtable_origin[i][j]->metric;
				lowdest = hashtable_origin[i][j]->dest;
                                low_index = i;
                                low_column=j;

			}			
		
		}
	}
	if(lowdest ==0)
		return ;
	hashtable_origin[low_index][low_column]->flag = 1 ;
}



int main(){
	int nodeNum = 3;
	TABLE*** imm;
	*p_path = 2010 ;
	p_path ++;
	imm = (TABLE***) malloc(sizeof(TABLE **) * BK);
	int i;
	for (i=0 ; i<BK ; i++)
	{
		imm[i] = (TABLE **)malloc(sizeof(TABLE) * SL);
	} 
	hashtable_origin = imm;
	FILE* fp ;
	char string[100];
	char addr[100];
	fp = fopen("input33.txt" , "r" ) ;
	printf("open file\n)");
	
	AddTable(fp,hashtable_origin);
	printTable("origin",hashtable_origin);
	findShortest();
	printf("lowdest : %d,lowmetric: %d  \n",lowdest,lowmetric);
	*p_path = lowdest;
	p_path ++;
	int k=0;
	for (k=11 ; k<23/* nodeNum-2*/ ; k= k +11)
	{	imm = (TABLE***) malloc(sizeof(TABLE **) * BK);
        	for (i=0 ; i<BK ; i++)
        	{
       			imm[i] = (TABLE **)malloc(sizeof(TABLE) * SL);
        	}
		hashtable_compare = imm; 
		sprintf(addr,"input%d.txt",k);
		fp_cmp= fopen(addr,"r");
		AddTable(fp_cmp,hashtable_compare);
	//{
		printTable("compare",hashtable_compare);
		calculate(hashtable_origin,hashtable_compare);
		printTable("updated",hashtable_origin);
		findShortest();
		*p_path =lowdest;
		p_path ++;
	}

	printf("dijstra path:");
        for(int* imm=path;*imm!=0;imm++)
        	printf("%d-",*imm);
	printf("\n");
	printTable("Final",hashtable_origin);
	return 0;

}


