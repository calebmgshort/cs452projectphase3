#include <usloss.h>
#include <usyscall.h>

#include "phase1.h"
#include "phase2.h"

int debugflag3 = 0;

int start2(char *arg)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start3(): Called.\n");
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
    systemArgs args;
    args.number = SYS_SPAWN;
    args.arg1 = (void *) start3;
    args.arg2 = NULL;
    args.arg3 = (void *) (4 * USLOSS_MIN_STACK);
    args.arg4 = (void *) 3;
    args.arg5 = (void *) "start3";
    USLOSS_Syscall((void *) &args);
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


