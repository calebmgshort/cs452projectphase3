#include <usloss.h>
#include <usyscall.h>

#include "phase1.h"
#include "phase2.h"
#include "phase3.h"

int debugflag3 = 0;

Semaphore[] semaphores = Semaphore[MAXSEMS];

int start2(char *arg)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Called.\n");
    }

    // Check kernel mode
    if (!(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE))
    {
        USLOSS_Console("start2(): Called from user mode.  Halting...\n");
    }

    // initialize system call vector
    for (int i = 0; i < MAXSYSCALL; i++)
    {
        systemCallVec[i] = nullsys3;
    }
    systemCallVec[SYS_SPAWN] = spawn;
    systemCallVec[SYS_WAIT] = wait;
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_SEMCREATE] = semCreate;
    systemCallVec[SYS_SEMP] = semP;
    systemCallVec[SYS_SEMV] = semV;
    systemCallVec[SYS_SEMFREE] = semFree;
    systemCallVec[SYS_GETTIMEOFDAY] = getTimeOfDay;
    systemCallVec[SYS_CPUTIME] = cpuTime;
    systemCallVec[SYS_GETPID] = getPID;

    // spawn start 3
    int pid = spawnReal("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);

    // wait for start3 to finish
    int status;
    pid = waitReal(&status);
}

void nullsys3(systemArgs *arg)
{
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

void cpuTime(systemArgs *args)
{
}

void getPID(systemArgs *args)
{
}
