/*
 * These are the definitions for phase 3 of the project
 */

#ifndef _PHASE3_H
#define _PHASE3_H

#define MAXSEMS         200

#define NO_SLOTS_AVAILABLE -1

typedef struct USLOSS_Sysargs
{
        int  number;
        void *arg1;
        void *arg2;
        void *arg3;
        void *arg4;
        void *arg5;
} systemArgs;

typedef struct
{
    int count;
} Semaphore;

#endif /* _PHASE3_H */
