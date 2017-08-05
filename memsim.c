#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int no_frms=0;    //Number of frames
int no_h=0,no_m=0; //Number of hits,misses
int no_dw=0,no_dr=0; //Diskwrites,Diskreads
int no_pf=0;      
int fst_pge = 1;    //First node that should be created
int mem_size=1;
int debug =0;

char* inp_file;   //Pointers to take arguments from command line
char* alg_nameorithm;
char* inp_mode;
//Defining a structure to represent frame and all its components
struct node{     
char dirty_flag;
unsigned pg_ref; //Baseaddress of the page
char scnd_ch;    //Second chance bit used in vms
struct node *nxt_node;
};

struct node *head_node=NULL,*new_node,*lst_node,*vms_pointer = NULL;

void showMemory(); //Function to display details of each event in debug mode
void vms(unsigned page,int write); //To perform VMS policy
void lru(unsigned page,int write); //To perform LRU policy

void main(int argc , char *argv[])
{
	if(argc!=2)  // argc should be 2 for correct execution
	printf("\n Using %s \n ",argv[0]);

	FILE *filepntr;
	char rw;
	char readline;
	unsigned page;
    
	int evnts=0;   //Declaring variables, num of events, rw counter 
	int write=0;
	unsigned linemem_refres; //Memory reference
	inp_file = argv[1];      //Reading the input from commandline
	no_frms = atoi(argv[2]);
	alg_nameorithm = argv[3];
	inp_mode = argv[4];
	if(strcmp(inp_mode,"debug")==0)debug=1; //To check if it is debug mode
	
	
	filepntr = fopen(inp_file,"readline"); //Opening trace file
	
	int alg_name = strcmp(alg_nameorithm,"vms"); //Which algo to do
	
	if(alg_name == 0){
	while((readline=fscanf(filepntr,"%x %c",&linemem_refres,&rw))!=EOF){
	page = linemem_refres>>12;
	if(debug) //debug mode statements
	 printf("Extracted Pageno -> %x \n",page);
    write=0;
	if(rw=='W')write=1;
	vms(page,write); //Calling function vms
	if(debug)  //debug mode statements
	showMemory();
	evnts = evnts+1;
	}
	}
	else {
	while((readline=fscanf(filepntr,"%x %c",&linemem_refres,&rw))!=EOF){
	page = linemem_refres>>12;
	if(debug) //debug mode statements
	printf("Extracted Pageno -> %x \n",page);
	write=0;
	if(rw=='W')write=1;
	lru(page,write); //Calling function lru
	evnts = evnts+1;
	if(debug) //debug statements
	showMemory();
	}
	}
	//Statements to be seen in ouput
	printf("Events in trace = %d \n",evnts);
	//printf("Frames in total = %d \n",no_frms);
	printf("Hits occured =%d \n", no_h);
	//printf("PageFaults =%d \n", no_m);
	//printf("HitRate is =%f \n",(no_h)/(no_h+no_m)*100);
	printf("Cache size is =%d \n",no_frms);
	printf("Diskwrites =%d\n", no_dw);
	printf("DiskReads =%d \n", no_dr);
	//Closing the trace file
	fclose(filepntr);
}
//To enter the frames in to the ds we use LRU_FrameIn
void LRU_FrameIn(unsigned page,int write){
	no_dr=no_dr+1; //Incrementing writes,reads,misses
	no_m=no_m+1;
	if(write)no_dw++;
	struct node *temp;
	new_node = (struct node *)malloc(sizeof(struct node));
	new_node->dirty_flag = write;
	new_node->pg_ref = page;
	if(head_node == NULL){
		fst_pge = 0;
		mem_size++;
		new_node->nxt_node = NULL;
		head_node = new_node;
		lst_node = head_node;
		
		}
	else{
		new_node->nxt_node = head_node;	
		head_node = new_node;
		lst_node->nxt_node=head_node;}
	
}
//To enter frames when vmu is performing
void VMS_FrameIn(unsigned page,int write){
	struct node *temp;
	no_dr=no_dr+1;
	no_m=no_m+1;
	if(write)no_dw++;
	
	new_node = (struct node *)malloc(sizeof(struct node));
	new_node->pg_ref = page;
	new_node->dirty_flag = write;
	new_node->scnd_ch = '0';
	if(head_node == NULL){
			mem_size = mem_size+1;
		fst_pge = 0;
		new_node->nxt_node = NULL;
		head_node = new_node;
		lst_node = head_node;
		
		
		}
	else{
		lst_node->nxt_node=new_node;
		lst_node = new_node;
		lst_node->nxt_node=head_node;}
}

bool isEmpty()//Function to check whether the list is empty
{
	if(fst_pge) return 1;
	else return 0;
}

//Function to verify the presence of a page 
struct node* fetch(unsigned page){
	struct node *temp;
	temp = head_node;
	struct node *curr_node=NULL;
	if(temp->pg_ref == page)
			curr_node = temp;
	else
	{
	temp=temp->nxt_node;
	for(;temp!=head_node && temp!= NULL;temp=temp->nxt_node)
	{
		if(temp->pg_ref == page)
			{
			curr_node = temp;
			if(debug) //debug statements
			printf("\n Reached if %x \n",temp->pg_ref);
			break;
			}
		else
			{
			if(debug)
			printf("\n Reached else %x \n",temp->pg_ref);
			}
	}
	}
	return curr_node;
}

void showMemory(){ //Displays event details in debug mode
	struct node *temp;
	temp=head_node;
	printf("\n Memory:\n");
	printf("%x %d %c ==>",temp->pg_ref,temp->dirty_flag,temp->scnd_ch);
	temp=temp->nxt_node;
	for(;temp!=head_node && temp!=NULL;temp=temp->nxt_node)
		printf("%x %d %c \n",temp->pg_ref,temp->dirty_flag,temp->scnd_ch);
	printf("\n=====================================\n");
}

//Performs LRU policy
void lru(unsigned page,int write){
	struct node *prev_node,*curr_node,*temp;
	if(isEmpty())
		LRU_FrameIn(page, write);
	else{
		curr_node = fetch(page);
		if(curr_node!=NULL && head_node!= NULL){
			no_h=no_h+1;  //Incrementing hits
			if(curr_node->dirty_flag == 0 && write)no_dw=no_dw+1; //Incrementing writes
			curr_node->dirty_flag=write;
			
			if(curr_node != head_node){
				for(temp = head_node;temp->nxt_node!=curr_node;temp=temp->nxt_node);
				prev_node = temp;
				if(curr_node->nxt_node==head_node)lst_node = prev_node;
				prev_node->nxt_node = curr_node->nxt_node;
				curr_node->nxt_node = head_node;
				head_node=curr_node;
				lst_node->nxt_node=head_node;
			}			
			}else{
			if(mem_size <= no_frms){
				LRU_FrameIn(page,write);
				mem_size=mem_size+1;
			}
			else {
				for(temp = head_node;temp->nxt_node->nxt_node!=head_node;temp=temp->nxt_node); 
				prev_node = temp;
				curr_node = prev_node->nxt_node;
				prev_node->nxt_node = head_node;
				lst_node = prev_node;
				free(curr_node);
				LRU_FrameIn(page,write);
				}}
		 }
}

//Perfoms vms algorithm
void vms(unsigned page,int write){
	struct node *prev_node,*curr_node;
	struct node *temp;
	if(isEmpty()){
	VMS_FrameIn(page, write);
	vms_pointer=head_node;
	}
	else{
	curr_node = fetch(page);
	if(curr_node!=NULL && head_node!= NULL)
	{
	
	if(curr_node->dirty_flag == 0 && write)no_dw=no_dw+1;
	curr_node->dirty_flag=write;
	curr_node->scnd_ch = '1'; //Pointing to second chance bit
	if(debug)
	printf("\n page already present\n ");
    no_h=no_h+1;}

	else{
	if(mem_size <= no_frms){
		mem_size=mem_size+1;
		VMS_FrameIn(page,write);} //Calls vms_framein function
	else {
		no_dr=no_dr+1;
		no_m=no_m+1;
		if(write)no_dw++;
		for(temp = vms_pointer;temp->scnd_ch!='0';temp=temp->nxt_node)
			temp->scnd_ch ='0';
		temp->pg_ref = page;
		temp->dirty_flag=write;
		temp->scnd_ch='0';
		vms_pointer = temp->nxt_node;}
	}
	}
}
