#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#define MAXNUM 10

typedef struct _Node{
    char tid[128];
    int mid;
    int key;
} Node;

int nodeNum = 0;
int waitNum = 0;

Node *locked_mutex[MAXNUM];
Node *waiting_mutex[MAXNUM];
// When input mutex is already locked, wait until it's unlocked

int cnt;
int edge[MAXNUM][MAXNUM];
int route[MAXNUM];
int routeNum = 0 ;

int isCycleUtil(int s, int *isvisited){
    route[routeNum++] = s;
    if(isvisited[s]==1)
	return 1;
    isvisited[s] = 1;
    for(int i = 0; i < MAXNUM; ++i)
	if(edge[s][i] == 1)
	    if(isCycleUtil(i, isvisited))
		return 1;
    route[--routeNum] = -1;
    return 0;
}

int isCycle(){
    for(int i = 0; i < MAXNUM; ++i){
	routeNum = 0;
	int isvisited[MAXNUM] = {0} ;
	if(isCycleUtil(i, isvisited))
	    return 1;
    }
    return 0;
}

void printGraph(){
    for(int i = 0; i < nodeNum; ++i){
	for(int j = 0; j < nodeNum; ++j){
	    printf("%d ", edge[i][j]) ;
	}
	printf("\n");
    }
}

void createNode(int mid, char tid[128]){
    Node* newNode = malloc(sizeof(struct _Node));
    newNode->mid = mid;
    strcpy(newNode->tid, tid);
    newNode->key = nodeNum;
    printf("New Node Created. Key is %d\n", newNode->key);
    locked_mutex[nodeNum++] = newNode;
}

int FindSameMutex(int mid){
    for(int i = 0; i < nodeNum; ++i){
	if(locked_mutex[i]->mid == mid)
	    return i;
    }
    return -1;
}

void connectEdge(int i, int mid, char tid[128]){
    for(int j = 0; j < nodeNum; ++j){
	if(strcmp(locked_mutex[j]->tid, tid)==0){
	    printf("Edge Created between %d - %d\n", locked_mutex[j]->key, locked_mutex[i]->key) ;
	    edge[locked_mutex[j]->key][locked_mutex[i]->key] = 1;
	}
    }
}

void option_lock(int mid, char tid[128]);
void option_unlock(int mid, char tid[128]);
void put_mutex_lock_on_the_waiting_list(int mid, char tid[128]) ;

void DeadlockDetected();

pid_t source, chck;

int 
main (int argc, char* argv[])
{
	chck = getpid();
	char cmd[256] = "LD_PRELOAD=\"./ddmon.so\" ./";
	strcat(cmd, argv[1]);
	source = fork();
	if(source == 0){
	    system(cmd);
	    sleep(1);
	    kill(chck, SIGKILL);
	    printf("-----Deadlock is not detected!-----\n");
	    return 0;
	}
	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;

	while (1) {
		char buf[128];

		char mutex_id[128];
		char tid[128];
		char option[1];

		int len = read(fd, buf, 128);
		if(len > 0) 
		{
		    char *temp = strtok(buf, " ");
		    strcpy(mutex_id, temp);
		    int mid = atoi(mutex_id); // char to int for making it easy to handle

		    temp = strtok(NULL, " ");
		    strcpy(tid, temp);

		    temp = strtok(NULL, " ");
		    strcpy(option, temp); // option 1. lock // option 2. unlock
		    int opt = atoi(option);

		    if(opt == 1){
			option_lock(mid, tid);
		    }
		    else if(opt == 2){
			option_unlock(mid, tid);
		    }
		    printGraph();
		}
	}
	close(fd) ;
	return 0 ;
}

void option_lock(int mid, char tid[128]){
    //printf("Locking %d mutex\n", mid) ;
    if(nodeNum == 0){
	createNode(mid, tid);
    }
    else{
	int mutex_index = FindSameMutex(mid); // If input mutex is not locked now, it returns -1
	if(mutex_index == -1) //if input mutex is not locked now
	    createNode(mid, tid);
	else { // If input mutex is locked now, it 'i' get index of mutex from locked mutex list
	    if(strcmp(locked_mutex[mutex_index]->tid, tid)==0){
		DeadlockDetected();
	    }
	    else{
		connectEdge(mutex_index, mid, tid) ;
		if(isCycle()){
		    DeadlockDetected();
		}
	    }
	    put_mutex_lock_on_the_waiting_list(mid, tid) ;
	}
    }
}

void put_mutex_lock_on_the_waiting_list(int mid, char tid[128]){
    Node* newNode = malloc(sizeof(struct _Node));
    newNode->mid = mid;
    strcpy(newNode->tid, tid);
    newNode->key = -1;
    waiting_mutex[waitNum++] = newNode ;
}
void callWaitingMutex(int mid){
    for(int i = 0; i < waitNum; ++i){
	if(waiting_mutex[i]->mid == mid){
	    printf("Waiting Mutex %d is called.\n", mid) ;
	    createNode(mid, waiting_mutex[i]->tid);
	    while(i < waitNum - 1){
		waiting_mutex[i] = waiting_mutex[i+1];
		i++;
	    }
	    waiting_mutex[i] = NULL;
	    waitNum--;
	    break;
	}
    }
}
void option_unlock(int mid, char tid[128]){
    //printf("UnLocking %d mutex\n", mid) ;
    int key, tailkey;
    for(int i = 0; i < nodeNum; ++i){
	if(locked_mutex[i]->mid == mid){
	    tailkey = locked_mutex[nodeNum-1]->key ;
	    key = locked_mutex[i]->key ;
	    locked_mutex[i] = locked_mutex[nodeNum-1] ;
	    locked_mutex[i]->key = key ;

	    break;
	}
	else if(i == nodeNum-1){
	    printf("%d - Corresponding mutex is not here.\n", mid);
	    return ;
	}
    }
    printf("key: %d, tkey: %d, num: %d\n", key, tailkey, nodeNum) ;
    for(int i = 0; i < nodeNum; ++i){
	edge[i][key] = edge[i][tailkey] ;
	edge[i][tailkey] = 0;
    }
    for(int i = 0; i < nodeNum; ++i){
	edge[key][i] = edge[tailkey][i] ;
	edge[tailkey][i] = 0 ;
    }
    printf("Key %d is unlocked.\n", key);
    
    nodeNum--;
    callWaitingMutex(mid) ;
}

void DeadlockDetected(){
    printf("Deadlock Detected\n") ;
    printf("----Below thread, mutex are involved in the deadlock----\n") ;

    for(int i = 0; i < routeNum-1; ++i)
	for(int j = 0; j < nodeNum; ++j)
	    if(locked_mutex[j]->key == route[i]){
		printf("| Thread ID: %s, Mutex memory address: %d |\n", locked_mutex[j]->tid, locked_mutex[j]->mid);
		break;
	    }
    printf("--------------------------------------------------------\n") ;
    kill(source, SIGKILL);
    exit(0);
}


