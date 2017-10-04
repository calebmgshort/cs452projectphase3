#include <usloss.h>

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

// TODO: There is some limit on semaphores. Fix this
Semaphore[] semaphores = Semaphore[50];

int debugflag3 = 0;

int start2(char *arg)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start3(): Called.\n");
    }

}

void spawn(systemArgs *arg)
{
}

void wait(systemArgs *args)
{
}

void terminate(systemArgs *args)
{
}

void semCreate(systemArgs *args)
{
    int initSemValue = *((int*) args->arg1)); // TODO: use val in pointer or pointer?
    int* args1Val = &(-1);
    if(initSemValue < 0)
    {
        args->arg1 = (void*) args1Val;
        args->arg4 = (void*) args1Val;
        return;
    }
    int handle = getAvailableHandle();  // TODO: implement
    if(hande == NO_SLOTS_AVAILABLE)
    {
        args->arg1 = (void*) args1Val;
        args->arg4 = (void*) args1Val;
        return;
    }
    *args1Val = handle;

    Semaphore *sem;
    sem->count = initSemValue
    semaphores[handle] = sem;
    args->arg1 = (void*) args1Val;
    args->arg4 = (void*) &(0);
    return;
}

void semP(systemArgs *args)
{
}

void semV(systemArgs *args)
{
}

void semFree(systemArgs *args)
{
}

void getTimeOfDay(systemArgs *args)
{
}

void CPUTime(systemArgs *args)
{
}

void getPID(systemArgs *args)
{
}
