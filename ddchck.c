#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#define MAXNUM 10

typedef struct _Node{
    char tid[128];
    int mid;
    int key;
} Node;

int nodeNum = 0;

Node *locked[MAXNUM];
int cnt;
int edge[MAXNUM][MAXNUM];

void printGraph(){
    for(int i = 0; i < nodeNum; ++i){
	for(int j = 0; j < nodeNum; ++j){
	    printf("%d ", edge[i][j]) ;
	}
	printf("\n");
    }
}

int 
main ()
{
	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;

	while (1) {
		int len;
		char buf[128];

		char mutexid[128];
		char tid[128];

		len = read(fd, buf, 128);
		if(len > 0) 
		{
		    char *temp = strtok(buf, " ");
		    strcpy(mutexid, temp);
		    int mid = atoi(mutexid);
		    temp = strtok(NULL, " ");
		    strcpy(tid, temp);
		    printf("mid: %d, tid: %s\n", mid, tid) ;
		    if(nodeNum == 0){
			Node* newNode = malloc(sizeof(struct _Node));
			newNode->mid = mid;
			strcpy(newNode->tid, tid);
			newNode->key = nodeNum;
			printf("New Node Created\n");
			locked[nodeNum++] = newNode;
		    }
		    else{
		    int found = 0;
		    for(int i = 0 ; i < nodeNum; ++i){
			printf("Compare %d : %d\n", locked[i]->mid, mid);
			if(locked[i]->mid == mid){
			    found = 1;
			    if(strcmp(locked[i]->tid, tid)==0){
				printf("Deadlock Detected!\n") ;
			    }
			    else{
				for(int j = 0; j < nodeNum; ++j){
				    if(strcmp(locked[j]->tid, tid)==0){
					printf("Edge Created! (%d, %d)\n", locked[j]->key, locked[i]->key) ;
					edge[locked[j]->key][locked[i]->key] = 1;
				    }
				}
			    }
			    break;
			}
		    }
		    if(found == 0){
			Node* newNode = malloc(sizeof(struct _Node));
			newNode->mid = mid;
			strcpy(newNode->tid, tid);
			newNode->key = nodeNum;
			printf("New Node Created 2\n");
			locked[nodeNum++] = newNode;
		    }
		    }
		    printGraph();
		}
	}
	close(fd) ;
	return 0 ;
}

/*
else{
    Node* newNode = malloc(sizeof(struct _Node));
    newNode->mid = mid;
    strcpy(newNode->tid, tid);
    newNode->key = nodeNum;
    printf("New Node Created 2\n");
    locked[nodeNum++] = newNode;
}*/
