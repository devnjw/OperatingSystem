#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *lock)
{
    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;

    void (*lockp)(void *) = NULL ;
    char * error ;

    lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
    if ((error = dlerror()) != 0x0)
	exit(1) ;
    
    long long pass = (long long)(lock);

    char buf[128];
    char temp[128];

    sprintf(buf, "%lld", pass) ;

    pthread_t tid = pthread_self();
    sprintf(temp, "%ud", (int)tid);
    strcat(buf, " ");
    strcat(buf, temp);
    strcat(buf, " 1"); // put '1' to let ddchck know this is 'lock' function.

    write(fd, buf, 128) ;
    close(fd) ;

    lockp(lock) ;

    return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *lock)
{
    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;

    void (*unlockp)(void *) = NULL ;
    char * error ;

    unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
    if ((error = dlerror()) != 0x0)
	exit(1) ;

    long long pass = (long long)(lock);

    char buf[128];
    char temp[128];

    sprintf(buf, "%lld", pass) ;

    pthread_t tid = pthread_self();
    sprintf(temp, "%ud", (int)tid);
    strcat(buf, " ");
    strcat(buf, temp);
    strcat(buf, " 2"); // put '2' to let ddchck know this is 'unlock' function.

    write(fd, buf, 128) ;
    close(fd) ;

    unlockp(lock) ;

    return 0;
}
