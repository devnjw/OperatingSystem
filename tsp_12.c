#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define MAXSIZE 50
#define MAXTASK 3 //Each process explore 12! routes

int arr[MAXSIZE][MAXSIZE] ;
int used[MAXSIZE] ;
int order[MAXSIZE+1] ;
int length = 0;
int N, count = 0;
int K;
int numP = 0 ;
int cnt = 0 ;

int pipes[2];

int best_order[MAXSIZE];
int min_length = 1000000;

void travel(int s, int num) ;
void print() ;
pid_t fork_child(int child_num);
void fileopen(char * fname);

int forkChild(int s, int num){    
    pid_t child_pid;
    
    if(pipe(pipes) != 0){
	perror("Error") ;
	exit(1) ;
    }

    if(cnt == K)
	wait(0);

    if(cnt < K){
	cnt++;
	numP++;
	child_pid = fork();
    }

    if(child_pid > 0){
	printf("Child %d is forked\n", child_pid);
	sleep(1);
	return 0;
    }
    else if(child_pid == 0){
	travel(s, num);
		

	//printf("length: %d, count: %d\n", min_length, count);
    	for(int i = 1; i <= N; ++i)
	    printf("%d-", best_order[i]);
	printf("%d length: %d\n", best_order[1], min_length);
	return 1;
    }
}

int distributeTasks(int s, int num){
    used[s] = 1;
    order[num] = s;
    
    int currP;

    // If condition satisfied, the remaining workload is 12!
    // Time to distribute the task to the Child Process.
    if(num == N - MAXTASK){
	//printf("Debug: s = %d, num = %d, cnt = %d, totalNum = %d\n", s, num, cnt, numP);
	currP = forkChild(s, num);
    }
    else{
	for(int i = 0; i < N; ++i){
	    if(used[i]!=1){
		length += arr[s][i];
		currP = distributeTasks(i, num+1);
		length -= arr[s][i];
	    }
	    if(currP==1)//If return process is child, break the loop.
		break;
	}
    }   
    
    used[s] = 0 ;
    order[num] = -1;
    
    return currP==1 ? 1 : 0;//If current process is Child return 1, if not return 0.
}

void sigchld_handler(int sig)
{
    int exitcode ;
    pid_t child = wait(&exitcode) ;
    printf("> child process %d is terminated with exitcode %d\n", child, WEXITSTATUS(exitcode)) ;

    cnt--;
}

int main(int argc, char* argv[]) {
    K = atoi(argv[2]);
    
    fileopen(argv[1]);
   
    print();

    int parent_pid = getpid();

    clock_t begin = clock();
    printf("Clock is started\n");

    signal(SIGCHLD, sigchld_handler) ;
    
    printf("N = %d\n", N);
    for(int i = 0; i < N; ++i)
	if(distributeTasks(i, 1)){
	    break;
	}

    if(getpid() == parent_pid){
	for(int i = 0; i < N%K; ++i)
	    wait(0x0);
	printf("Total number of forked child precess is %d\n", numP);

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC ;
	printf("Execution time : %f\n", time_spent);
    }
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
	    if(length > min_length)
		break;
	    if(used[i]!=1){
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

void print(){
    for(int i = 0; i < N; ++i){
	for(int j = 0; j < N; ++j){
	    printf("%d ", arr[i][j]);
	}
    printf("\n");
    }
}
