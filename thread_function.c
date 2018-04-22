#include<stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
	int dest;
	int link;
	int metric;
	int flag;
}TABLE;

TABLE origin[10]={0};
TABLE compare[10]={0};
TABLE*  point_origin= origin;
TABLE*  point_compare= compare;
int lowmetric=0;
int lowdest=0;
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
				check=1;
				break;
			}
		}
		if(check==1)
			continue;
		else
		{
			for( ;origin->dest != compare->dest; origin++)
			{	
				if(origin->dest==0)
					break;
			}
			if(origin->dest==0)
			{
				origin->dest = compare->dest;
				origin->link = compare->link;
				origin->metric= compare->metric;
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

int main(void)
{
	FILE* fp_org;
	FILE* fp_cmp;
	fp_org = fopen("input2.txt","r");
	ReadNInsert(fp_org, point_origin);
	TABLE* immpt;
	TABLE* lowpt;
	int path[10]={0};
	int*  p_path;
	p_path= path;
	*p_path = 88; // store itself
	p_path ++;
	printTable("initial",origin);

 	for (immpt = origin; immpt ->dest !=0 ; immpt++)   // until end of file 
	{
  		if (lowmetric == 0)		// the first time fo comparision --> initialize low dest and metric
  		{
  			lowmetric = immpt->metric;
   			lowdest = immpt->dest;
  		}
 		 if (lowmetric > immpt->metric)  // if there is dest which has lower metric then change value
  		{
			lowpt= immpt;
   			lowmetric = immpt->metric;
   			lowdest = immpt->dest;
  		}
 	}	

	lowpt->flag = 1;
	printf("low dest : %d \n",lowdest);

 	*p_path = lowdest;
	 p_path++;
	
	//request comm to lowdest
	fp_cmp = fopen("input3.txt","r");
	ReadNInsert(fp_cmp,point_compare);

	printf("read 2 complete\n");


	calculate(origin,compare,path);
	printTable("updated",origin);
	memset(&compare,0,sizeof(compare));

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
	printf("low dest : %d \n",lowdest);
	*p_path = lowdest;
        p_path++;
	fp_cmp = fopen("input.txt","r");
	printf("open file well \n");
        ReadNInsert(fp_cmp,point_compare);
	printTable("comopare1",compare);
        printf("read 3 complete\n");


        calculate(origin,compare,path);

        printTable("updated",origin);





	return 0;
}
