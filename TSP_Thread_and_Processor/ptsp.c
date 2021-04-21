#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define MAXSIZE 50
#define MAXTASK 12 //Each process explore 12! routes

int arr[MAXSIZE][MAXSIZE] ;
int used[MAXSIZE] ;
int order[MAXSIZE+1] ;
int best_order[MAXSIZE] ;

int length = 0 ;
int min_length = 1000000 ;
int N, K ; //N: Num of Nodes, K: Max num of child prcess
long long count = 0 ; //The number of traveled routes
int numP = 0 ; //The number of forked child processes
int cnt = 0 ; //The number of running child processes
int parent_pid ; //Store parent process pid

void distributeTasks(int s, int num) ;
void forkChild(int s, int num) ;
void travel(int s, int num) ;

void sigint_handler(int sig) ;
void sigchld_handler(int sig) ;
void kill_childs() ;

int pipe1[2] ;
void parent_read_pipe() ;
void child_write_pipe() ;

void print_solution() ; //Print out current best solution
void print() ; //Print out Graph

int curr_cpid[50] ;
void push_cpid() ;
void del_cpid() ;

void fileopen(char * fname) ;
int factorial(int num) ;


int main(int argc, char* argv[]) {
    K = atoi(argv[2]);
    fileopen(argv[1]);
    print();
    parent_pid = getpid() ;

    clock_t begin = clock();

    signal(SIGCHLD, sigchld_handler) ;
    signal(SIGINT, sigint_handler) ;
    
    for(int i = 0; i < N; ++i)
	distributeTasks(i, 1) ;

    for(int i = 0; i < N%K; ++i)
	wait(0x0);
    
    if(getpid()==parent_pid)
	print_solution() ;
 
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC ;
    printf("Execution time : %f\n", time_spent);
}

void distributeTasks(int s, int num){
    used[s] = 1;
    order[num] = s;

    // If condition satisfied, the remaining workload is 12!
    // Time to distribute the task to the Child Process.
    if(num == N - MAXTASK){
	forkChild(s, num);
    }
    else{
	for(int i = 0; i < N; ++i){
	    if(used[i]!=1){
		length += arr[s][i];
		distributeTasks(i, num+1);
		length -= arr[s][i];
	    }
	}
    }   
    
    used[s] = 0 ;
    order[num] = -1;
}

void forkChild(int s, int num){    
    if(pipe(pipe1) != 0){
	perror("Error") ;
	exit(1) ;
    }

    if(cnt == K)
	wait(0);

    pid_t child_pid;
    if(cnt < K){
	cnt++;
	numP++;
	child_pid = fork();
    }

    if(child_pid > 0){
	//printf("Child %d is forked\n", child_pid);
	push_cpid(child_pid);
	parent_read_pipe() ;
    }
    else if(child_pid == 0){
	count = 0;
	travel(s, num);

	kill(getpid(), SIGINT) ;
    }
}

void travel(int s, int num){
    used[s] = 1 ;
    order[num] = s;

    if(num==N){
	count++;
	length += arr[s][order[1]];

	if (length < min_length){
	    min_length = length;
	    memcpy(best_order+1, order+1, sizeof(int)*(N));
	}
	length -= arr[s][order[1]];
    }
    else{
	for(int i = 0; i < N; ++i){
	    if(length >= min_length){
		count += factorial(N-num);
		break;
	    }
	    else if(used[i]!=1){
		length += arr[s][i];
		travel(i, num+1);
		length -= arr[s][i];
	    }
	}
    }   

    used[s] = 0 ;
    order[num] = -1;
}

//---------------< Signal Handlers >---------------//
void sigchld_handler(int sig)
{
    pid_t child = wait(0x0) ;
    //printf("> child process %d is terminated\n", child) ;
    
    //parent_read_pipe() ;

    del_cpid(child) ;
    cnt--;
}

void sigint_handler(int sig)
{
    if(getpid() == parent_pid){
	kill_childs() ;	
	print_solution() ;
    }
    else{
	//printf("I'm a child %d, I want to write on pipe\n", getpid()) ;
	//print_solution() ;
	child_write_pipe() ;
    }
    exit(0);
}

void kill_childs()
{
    parent_read_pipe() ;
    for(int i = 0; i < K; ++i){
	printf("Kill process %d\n", curr_cpid[i]);
	if(curr_cpid[i]>0)
	    kill(curr_cpid[i], SIGINT) ;
	wait(0x0);
    }
}

//---------------< Pipe Operators >---------------//
void parent_read_pipe()
{
    long long tmp;
    int child_length;
    int tmpArr[MAXSIZE+1];

    close(pipe1[1]) ;
    read(pipe1[0], &child_length, sizeof(child_length)) ;
    read(pipe1[0], &tmp, sizeof(tmp)) ;
    for(int i = 1; i <= N; ++i)
	read(pipe1[0], &tmpArr[i], sizeof(tmpArr[i])) ;
    close(pipe1[0]) ;

    count += tmp ;
    if(min_length > child_length){
	min_length = child_length ;
	memcpy(best_order+1,tmpArr+1, sizeof(int)*N) ;

	printf("-----Updeted Status!-----\n") ;
	print_solution() ;
    }
}

void child_write_pipe()
{
    close(pipe1[0]) ;
    write(pipe1[1], &min_length, sizeof(min_length)) ;
    write(pipe1[1], &count, sizeof(count)) ;
    for(int i = 1; i <= N; ++i)
	write(pipe1[1], &best_order[i], sizeof(best_order[i])) ;
    close(pipe1[1]) ;
}


//---------------< Utilites >---------------//
void print_solution(){
    //Print out Solution
    printf("The best solution route: [");
    for(int i = 1; i <= N; ++i)
	printf("%d-", best_order[i]);
    printf("%d]\nThe length: %d\n", best_order[1], min_length);
    printf("The total number of traveled routes: %lld\n", count);
}

void print(){
    printf("-----Input Graph-----\n") ;
    for(int i = 0; i < N; ++i){
	for(int j = 0; j < N; ++j){
	    printf("%d ", arr[i][j]);
	}
    printf("\n");
    }
}

void push_cpid(int cpid)
{
    for(int i = 0; i < K; ++i)
	if(curr_cpid[i]==0){
	    curr_cpid[i] = cpid;
	    break;
	}
}
void del_cpid(int cpid)
{
    for(int i = 0; i < K; ++i)
	if(curr_cpid[i]==cpid)
	    curr_cpid[i]=0;
}

void fileopen(char * fname){
    FILE * fp = fopen(fname, "r") ;

    char tmp;
    while(fscanf(fp, "%c", &tmp)!=EOF)
	if(tmp=='\n')
	    N++;
    printf("Number of Nodes: %d\n", N);
   
    int temp; 
    rewind(fp);
    for (int i = 0 ; i < N ; i++) {
        for (int j = 0 ; j < N ; j++) {
	    fscanf(fp, "%d", &temp) ;
	    arr[i][j] = temp ;
	}
    }
    fclose(fp) ;
}

int factorial(int num)
{
    if( num == 0)
	return 1;
    return num*factorial(num-1);
}

