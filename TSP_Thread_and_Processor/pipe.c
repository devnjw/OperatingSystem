#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int pipes[2] ;
int length = 0;

void pipe_write(int i)
{
    close(pipes[0]) ;

    length = i;
    write(pipes[1], &length, sizeof(length));

    close(pipes[1]) ;
    
    //printf("Process number %d is closing\n", i);
}

void pipe_read()
{
    close(pipes[1]) ;

    read(pipes[0], &length, sizeof(length)) ;
    printf("Length = %d\n", length) ;

    close(pipes[0]) ;
}

void handler(int sig)
{
    int exitcode ;
    pid_t child = wait(&exitcode) ;
    printf("> child %d is terminated with exitcode %d\n", child, exitcode) ;
}

int main()
{
    signal(SIGCHLD, handler) ;
	
    pid_t child_pid ;
    int exit_code ;

    //pipe(pipes);
    
    for(int i = 1; i <= 8; ++i){
	pipe(pipes) ;

	child_pid = fork();
	if(child_pid == 0) {
	    pipe_write(i) ;
	    sleep(5);
	    break;
	}
	else if(child_pid > 0){
	    printf("child %d is forked\n", child_pid) ;
	    pipe_read();
	}
    }
    wait(&exit_code) ;

    exit(0) ;
}
