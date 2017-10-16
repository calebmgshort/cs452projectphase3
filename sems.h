#ifndef _PHASE3PROC_H
#define _PHASE3PROC_H

#include <usloss.h>
#include <usyscall.h>

#define DEBUG3 1

typedef struct userProc  * userProcPtr;
typedef struct userProc    userProc;
typedef struct semaphore * semaphorePtr;
typedef struct semaphore   semaphore;
typedef USLOSS_Sysargs     systemArgs;

struct userProc
{
    int pid;                  // The pid of this process
    int (*startFunc)(char *); // The function that spawnLaunch will call
    char *args;               // The arguments to the startFunc
};

struct semaphore
{
    int count;
};

#define NO_SLOTS_AVAILABLE -1

#endif
