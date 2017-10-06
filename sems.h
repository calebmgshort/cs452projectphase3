#ifndef _PHASE3PROC_H
#define _PHASE3PROC_H

typedef struct userProc  * userProcPtr;
typedef struct userProc    userProc;
typedef struct semaphore * semaphorePtr;
typedef struct semaphore   semaphore;

struct userProc
{
    int pid;                  // The pid of this process
    int (*startFunc)(char *); // The function that spawnLaunch will call
};

struct semaphore
{
    int count;
};

#define NO_SLOTS_AVAILABLE -1

#endif
