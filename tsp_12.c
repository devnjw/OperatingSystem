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
int length = 0;
int N, count = 0;
int K;
int numP = 0 ;
int cnt = 0 ;

int pipe1[2];
int pipe2[2];

int best_order[MAXSIZE];
int min_length = 1000000;

void print_solution() ;
void travel(int s, int num) ;
void print() ;
pid_t fork_child(int child_num);
void fileopen(char * fname);
int factorial(int num);

void write_pipe()
{
    close(pipe1[0]) ;
    write(pipe1[1], &min_length, sizeof(length)) ;
    close(pipe1[1]) ;
}

int read_pipe()
{
    int parent_length;
    close(pipe1[1]) ;
    read(pipe1[0], &parent_length, sizeof(parent_length)) ;
    close(pipe1[0]) ;
    return parent_length;
}

void parent_read_pipe()
{
    int tmp;
    int child_length;
    int tmpArr[MAXSIZE+1];

    close(pipe2[1]) ;
    read(pipe2[0], &child_length, sizeof(child_length)) ;
    read(pipe2[0], &tmp, sizeof(tmp)) ;
    for(int i = 1; i <= N; ++i)
	read(pipe2[0], &tmpArr[i], sizeof(tmpArr[i])) ;
    close(pipe2[0]) ;

    count += tmp ;
    if(min_length > child_length){
	min_length = child_length ;
	memcpy(best_order+1,tmpArr+1, sizeof(int)*N) ;
	printf("Updated Status! Min Length: %d\nBest Order: ", min_length) ;
	for(int i = 1; i <= N; ++i)
	    printf("%d-",best_order[i]) ;
	printf("%d\n",best_order[1]) ;
	
    }
}

void child_write_pipe()
{
    close(pipe2[0]) ;
    write(pipe2[1], &min_length, sizeof(min_length)) ;
    write(pipe2[1], &count, sizeof(count)) ;
    for(int i = 1; i <= N; ++i)
	write(pipe2[1], &best_order[i], sizeof(best_order[i])) ;
    close(pipe2[1]) ;
}

void forkChild(int s, int num){    
    pid_t child_pid;
    
    if(pipe(pipe1) != 0){
	perror("Error") ;
	exit(1) ;
    }
    pipe(pipe2) ;

    if(cnt == K)
	wait(0);

    if(cnt < K){
	cnt++;
	numP++;
	child_pid = fork();
    }

    if(child_pid > 0){
	printf("Child %d is forked\n", child_pid);
	//sleep(1);

	write_pipe();
	parent_read_pipe() ;
    }
    else if(child_pid == 0){
	count = 0;
	travel(s, num);
	
	child_write_pipe();
/*
    	for(int i = 1; i <= N; ++i)
	    printf("%d-", best_order[i]);
	printf("%d length: %d\n", best_order[1], min_length);
	*/exit(0) ;
    }
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

void sigchld_handler(int sig)
{
    int exitcode ;
    pid_t child = wait(&exitcode) ;
    printf("> child process %d is terminated with exitcode %d\n", child, WEXITSTATUS(exitcode)) ;
    cnt--;

    //close(pipes[0]) ;
    //write(pipes[1], &min_length, sizeof(min_length)) ;
    //close(pipes[1]) ;
}

void kill_childs()
{
    //for(int i = 0; i < K; ++i)
	//kill(curr_cpid[i], SIGKILL) ;
}

void sigint_handler(int sig)
{
    //kill_childs();
	    
    //Print out Solution
    printf("The best solution route: [");
    for(int i = 1; i <= N; ++i)
        printf("%d-", best_order[i]);
    printf("%d], length: %d\n", best_order[1], min_length);
    printf("Total number of forked child precess is %d\n", numP);

    exit(0);
}

int main(int argc, char* argv[]) {
    K = atoi(argv[2]);
    fileopen(argv[1]);
    print();

    clock_t begin = clock();

    signal(SIGCHLD, sigchld_handler) ;
    signal(SIGINT, sigint_handler) ;
    
    for(int i = 0; i < N; ++i)
	distributeTasks(i, 1) ;

    for(int i = 0; i < N%K; ++i)
	wait(0x0);

    print_solution() ;
 
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC ;
    printf("Execution time : %f\n", time_spent);
}
void print_solution(){
   //Print out Solution
    printf("The best solution route: [");
    for(int i = 1; i <= N; ++i)
	printf("%d-", best_order[i]);
    printf("%d], length: %d\n", best_order[1], min_length);
    printf("Total number of forked child precess is %d\n", numP);
}

pid_t fork_child(int child_num){
    pid_t child_pid ;
    child_pid = fork();
    
    return child_pid;
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

void print(){
    for(int i = 0; i < N; ++i){
	for(int j = 0; j < N; ++j){
	    printf("%d ", arr[i][j]);
	}
    printf("\n");
    }
}
