#include <usloss.h>
#include <usyscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include "sems.h"
#include "libuser.h"
#include "phase3utility.c"

// Debugging flag
int debugflag3 = 0;

// The sems table
semaphore Semaphores[MAXSEMS];

// The phase 3 proc table
userProc ProcTable[MAXPROC];

// Mutex box for the proc table
int ProcTableMutex;

// Prototypes for syscall handlers
void spawn(systemArgs *);
void wait(systemArgs *);
void terminate(systemArgs *);
void semCreate(systemArgs *);
void semP(systemArgs *);
void semV(systemArgs *);
void semFree(systemArgs *);
void getTimeOfDay(systemArgs *);
void cpuTime(systemArgs *);
void getPID(systemArgs *);
void nullsys3(systemArgs *);

// Prototypes for "Real" functions
int spawnReal(char *, int (*)(char *), char *, int, int);
int waitReal(int *);
int semCreateReal(int);
void terminateReal(int);

// Other prototypes
extern int start3(char *);
int spawnLaunch(char *);
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

    // Initialize system call vector
    for (int i = 0; i < MAXSYSCALLS; i++)
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

    // Create the mutex mailbox for the proc table
    ProcTableMutex = MboxCreate(1, 0);

    // Create first user-level process and wait for it to finish.
    // Here, we only call spawnReal(), since we are already in kernel mode.
    int pid = spawnReal("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);
    if (pid < 0)
    {
        USLOSS_Console("start2(): Could not create start3().  Halting...\n");
        USLOSS_Halt(1);
    }

    // Wait for start3 to finish
    int status;
    int result = waitReal(&status);
    if (result != pid)
    {
        USLOSS_Console("start2(): wait return a process other than start3().  Halting...\n");
        USLOSS_Halt(1);
    }
    return 0;
}

// TODO: Implement
void nullsys3(systemArgs *arg)
{
}

void spawn(systemArgs *arg)
{
    if (arg->number != SYS_SPAWN)
    {
        USLOSS_Console("spawn(): Called with wrong syscall number.\n");
        USLOSS_Halt(1);
    }

    int (*startFunc)(char *) = (int (*)(char *)) arg->arg1;
    char *startArgs = (char *) arg->arg2;
    int stackSize = (int) ((long) arg->arg3);
    int priority = (int) ((long) arg->arg4);
    char *name = (char *) arg->arg5;

    long result = spawnReal(name, startFunc, startArgs, stackSize, priority);
    arg->arg1 = (void *) result;
    arg->arg4 = (void *) 0;
    if (result < 0)
    {
        if (DEBUG3 && debugflag3)
        {
            USLOSS_Console("spawn(): call to spawnReal did not create a process.\n");
        }
        arg->arg1 = (void *) -1;
    }
    if (result == -2)
    {
        if (DEBUG3 && debugflag3)
        {
            USLOSS_Console("spawn(): arguments were invalid.\n");
        }
        arg->arg4 = (void *) -1;
    }
}

// TODO: Implement
void wait(systemArgs *args)
{
}

// TODO: Implement
void terminate(systemArgs *args)
{
}

// TODO: Implement
void semCreate(systemArgs *args)
{
    int initSemValue = (int) ((long) args->arg1);
    if(initSemValue < 0)    // Initial Sem value is invalid
    {
        args->arg4 = (void *) -1;
        return;
    }
    if(!isSemAvailable()) // TODO: Implement this function
    {
        args->arg4 = (void *) -1;
        return;
    }

    long semID = semCreateReal(initSemValue);

    args->arg1 = (void *) semID;
    args->arg4 = (void *) 0;

    if(isZapped())
    {
        terminateReal(0);  // TODO: does terminate take an input?
    }
    setToUserMode();
}

// TODO: Implement
void semP(systemArgs *args)
{
}

// TODO: Implement
void semV(systemArgs *args)
{
}

// TODO: Implement
void semFree(systemArgs *args)
{
}

// TODO: Implement
void getTimeOfDay(systemArgs *args)
{
}

// TODO: Implement
void cpuTime(systemArgs *args)
{
}

// TODO: Implement
void getPID(systemArgs *args)
{
}

/******************** Real Functions ******************/
/*
 * Calls fork1 to create a process running spawnLaunch with the supplied args.
 * spawnLaunch then changes into user mode and calls the supplied function.
 * Returns -2 if an argument is invalid. Returns -1 if all args were valid, yet
 * no process was created. Returns the pid of the created proc otherwise. Must
 * be called in kernel mode for fork1 to complete.
 */
int spawnReal(char *name, int (*startFunc)(char *), char *args, int stackSize, int priority)
{
    // Check params
    if (name == NULL || strlen(name) >= (MAXNAME - 1))
    {
        return -2;
    }
    if (stackSize < USLOSS_MIN_STACK)
    {
        return -2;
    }
    if (strlen(args) >= (MAXARG - 1))
    {
        return -2;
    }
    if (priority < 1 || priority > 5)
    {
        return -2;
    }
    if (startFunc == NULL)
    {
        return -2;
    }

    // Create a mailbox for synchronization with spawnLaunch
    int mbox = MboxCreate(0, MAX_MESSAGE);

    // Convert the mboxID into a string
    char buf[MAX_MESSAGE];
    sprintf(buf, "%d", mbox);

    // Fork a new process
    int pid = fork1(name, spawnLaunch, buf, stackSize, priority);
    if (pid < 0)
    {
        MboxRelease(mbox);
        return -1;
    }

    // Setup the proc table for the new proc
    lock(ProcTableMutex);
    userProcPtr proc = &ProcTable[pid % MAXPROC];
    proc->pid = pid;
    proc->args = args;
    proc->startFunc = startFunc;
    unlock(ProcTableMutex);

    // Let spawnLaunch execute and release the mailbox
    MboxSend(mbox, NULL, 0);
    MboxRelease(mbox);

    // Return the pid of the new proc
    return pid;
}

int spawnLaunch(char *arg)
{
    // Unpack the mboxID from args
    int mboxID = atoi(arg);

    // Wait for spawnReal to finish setting up the proc table
    MboxReceive(mboxID, NULL, 0);

    // Change the process into user mode
    setToUserMode();

    // Call the start func and capture return value
    lock(ProcTableMutex);
    userProcPtr proc = &ProcTable[getpid() % MAXPROC];
    int status = proc->startFunc(proc->args);
    unlock(ProcTableMutex);

    // Terminate
    Terminate(status);
    return 0;
}

// TODO: Implement
int waitReal(int *status)
{
    return -1;
}

// TODO: Implement
void terminateReal(int status)
{
}

// TODO: Implement
int semCreateReal(int initSemValue)
{
    int semHandle = getAvailableSemHandle();  // TODO: implement
    initSem(semHandle, initSemValue);         // TODO: implement

    // TODO: mailbox stuff

    return semHandle; // The handle is just the semID
}

// TODO: Implement
void semPReal()
{
}

// TODO: Implement
void semVReal()
{
}

// TODO: Implement
void semFreeReal()
{
}

// TODO: Implement
void getTimeOfDayReal()
{
}

// TODO: Implement
void cpuTimeReal()
{
}

// TODO: Implement
void getPIDReal()
{
}
