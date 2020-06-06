#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>

int
pthread_mutex_lock(pthread_mutex_t *lock)
{
    printf("Mutex lock in\n");
    void (*lockp)(void *) = NULL ;
    char * error ;

    lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
    if ((error = dlerror()) != 0x0)
	exit(1) ;

    lockp(lock) ;
    printf("Mutex lock out\n") ;

    return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *lock)
{
    printf("Mutex unlock in\n");
    void (*unlockp)(void *) = NULL ;
    char * error ;

    unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
    if ((error = dlerror()) != 0x0)
	exit(1) ;

    unlockp(lock) ;
    printf("Mutex unlock out\n") ;

    return 0;
}
