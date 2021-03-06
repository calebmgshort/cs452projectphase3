#include <usloss.h>
#include <usyscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include "sems.h"
#include "phase3utility.h"
#include "libuser.h"

// Debugging flag
int debugflag3 = 0;

// The sems table
semaphore Semaphores[MAXSEMS];
int currentNumSems = 0;
int semsMutex;

// The phase 3 proc table
userProc ProcTable[MAXPROC];

// Mutex box for the proc table
int ProcTableMutex;

// Prototypes for syscall handlers
void spawn(systemArgs *);
void waitHandler(systemArgs *);
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
void semPReal(int);
void semVReal(int);
int semFreeReal(int);
void terminateReal(int);
int getTimeOfDayReal();
int cpuTimeReal();
int getPIDReal();

// Other prototypes
extern int start3(char *);
int spawnLaunch(char *);

/*
 * Entry point for phase3. Initializes the system call handlers and tables.
 */
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
        USLOSS_Halt(1);
    }

    // Initialize system call vector
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Initializing syscall vector.\n");
    }
    for (int i = 0; i < MAXSYSCALLS; i++)
    {
        systemCallVec[i] = nullsys3;
    }
    systemCallVec[SYS_SPAWN] = spawn;
    systemCallVec[SYS_WAIT] = waitHandler;
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_SEMCREATE] = semCreate;
    systemCallVec[SYS_SEMP] = semP;
    systemCallVec[SYS_SEMV] = semV;
    systemCallVec[SYS_SEMFREE] = semFree;
    systemCallVec[SYS_GETTIMEOFDAY] = getTimeOfDay;
    systemCallVec[SYS_CPUTIME] = cpuTime;
    systemCallVec[SYS_GETPID] = getPID;

    // Initialize the semaphore table and related items
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Initializing semaphore table.\n");
    }
    genericSemaphoreInitialization();

    // Initialize the ProcTable
    for (int i = 0; i < MAXPROC; i++)
    {
        ProcTable[i].pid = EMPTY;
    }

    // Create the mutex mailbox for the proc table
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Creating mailboxes for mutual exclusion.\n");
    }
    ProcTableMutex = MboxCreate(1, 0);

    // Create first user-level process and wait for it to finish.
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Spawning start3 in user mode.\n");
    }

    // Here, we only call spawnReal(), since we are already in kernel mode.
    int pid = spawnReal("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);
    if (pid < 0)
    {
        USLOSS_Console("start2(): Could not create start3().  Halting...\n");
        USLOSS_Halt(1);
    }

    // Wait for start3 to finish
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("start2(): Waiting for start3().\n");
    }
    int status;
    int result = waitReal(&status);
    if (result != pid)
    {
        USLOSS_Console("start2(): Wait returned a process other than start3().  Halting...\n");
        USLOSS_Halt(1);
    }
    return 0;
}

/*
 * Syscall Handler for undefined syscalls. Terminates the calling process.
 */
void nullsys3(systemArgs *arg)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("nullsys3(): Called\n");
    }
    terminateReal(1);
}

/*
 * Syscall handler for Spawn. Defers to spawnReal.
 */
void spawn(systemArgs *arg)
{
    // Check the syscall number
    if (arg->number != SYS_SPAWN)
    {
        USLOSS_Console("spawn(): Called with wrong syscall number.\n");
        USLOSS_Halt(1);
    }

    // Unpack the args
    int (*startFunc)(char *) = (int (*)(char *)) arg->arg1;
    char *startArgs = (char *) arg->arg2;
    int stackSize = (int) ((long) arg->arg3);
    int priority = (int) ((long) arg->arg4);
    char *name = (char *) arg->arg5;

    // Call spawnReal
    long result = spawnReal(name, startFunc, startArgs, stackSize, priority);

    // Unpack the return values
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

    // Check if we've been zapped
    if (isZapped())
    {
        terminateReal(0);
    }

    // Set to user mode
    setToUserMode();
}

/*
 * Syscall Handler for Wait. Defers to waitReal.
 */
void waitHandler(systemArgs *args)
{
    // Check the syscall number
    if (args->number != SYS_WAIT)
    {
        USLOSS_Console("wait(): Called with wrong syscall number.\n");
        USLOSS_Halt(1);
    }

    // Call waitReal
    int status;
    long result = waitReal(&status);

    // Unpack the return values
    args->arg1 = (void *) result;
    args->arg2 = (void *) ((long) status);

    // Check if we've been zapped
    if (isZapped())
    {
        terminateReal(0);
    }

    // Set to user mode
    setToUserMode();
}

/*
 * Syscall Handler for Terminate. Defers to terminateReal
 */
void terminate(systemArgs *args)
{
    // Check the syscall number
    if (args->number != SYS_TERMINATE)
    {
        USLOSS_Console("terminate(): Called with wrong syscall number.\n");
        USLOSS_Halt(1);
    }

    // Unpack arguments
    int status = (int) ((long) args->arg1);

    // Call terminateReal
    terminateReal(status);
}

/*
 *  The syscallHandler for SemCreate
 */
void semCreate(systemArgs *args)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semCreate(): called\n");
    }

    int initSemValue = (int) ((long) args->arg1);
    // Error checking
    if(initSemValue < 0)    // Initial Sem value is invalid
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semCreate(): The given initial sem value %d is invalid\n", initSemValue);
        }
        args->arg4 = (void*) -1;
        return;
    }
    if(!isSemAvailable())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semCreate(): There are no more semaphores available\n");
        }
        args->arg4 = (void *) -1;
        return;
    }

    // Normal case
    int semID = semCreateReal(initSemValue);

    args->arg1 = (void *) ((long) semID);
    args->arg4 = (void *) 0;

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semCreate(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

/*
 *  The syscallHandler for SemP
 */
void semP(systemArgs *args)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semP(): called\n");
    }
    int semHandle = (int) ((long) args->arg1);
    // Error checking
    if(!doesGivenSemExist(semHandle))
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semP(): The semaphore with id %d does not exist\n", semHandle);
        }
        args->arg4 = (void*) -1;
        return;
    }

    // Normal case
    semPReal(semHandle);

    args->arg4 = (void*) 0;

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semP(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

/*
 *  The syscallHandler for SemV
 */
void semV(systemArgs *args)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semV(): called\n");
    }
    int semHandle = (int) ((long) args->arg1);
    // Error checking
    if(!doesGivenSemExist(semHandle))
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semV(): The semaphore with id %d does not exist\n", semHandle);
        }
        args->arg4 = (void*) -1;
        return;
    }

    // Normal case
    semVReal(semHandle);

    args->arg4 = (void*) 0;

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semV(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

/*
 *  The syscallHandler for SemFree
 */
void semFree(systemArgs *args)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semFree(): called\n");
    }

    int semHandle = (int) ((long) args->arg1);
    // Error checking
    if(!doesGivenSemExist(semHandle))
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semFree(): The semaphore with id %d does not exist\n", semHandle);
        }
        args->arg4 = (void*) -1;
        return;
    }

    // Normal case
    int status = semFreeReal(semHandle);

    args->arg4 = (void*) ((long) status);

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semFree(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

/*
 *  The syscallHandler for GetTimeOfDay
 */
void getTimeOfDay(systemArgs *args)
{
    int timeOfDay = getTimeOfDayReal();

    args->arg1 = (void*) ((long) timeOfDay);

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("getTimeOfDay(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

/*
 *  The syscallHandler for cpuTime
 */
void cpuTime(systemArgs *args)
{
    int cputime = cpuTimeReal();

    args->arg1 = (void*) ((long) cputime);

    if(isZapped())
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("getTimeOfDay(): process was zapped\n");
        }
        terminateReal(0);
    }
    setToUserMode();
}

void getPID(systemArgs *args)
{
    // Check syscall number
    if (args->number != SYS_GETPID)
    {
        USLOSS_Console("getPID(): Called with wrong syscall number.\n");
        USLOSS_Halt(1);
    }

    // Call getPIDReal
    int pid = getPIDReal();

    // Put return values into args
    args->arg1 = (void *) ((long) pid);

    // Check if zapped
    if (isZapped())
    {
        terminateReal(0);
    }

    // Set to user mode
    setToUserMode();
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
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnReal(): Called.\n");
    }

    // Check params
    if (name == NULL || strlen(name) >= (MAXNAME - 1))
    {
        return -2;
    }
    if (stackSize < USLOSS_MIN_STACK)
    {
        return -2;
    }
    if (args != NULL && strlen(args) >= (MAXARG - 1))
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
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnReal(): Creating a mailbox for communication with spawnLaunch.\n");
    }
    int mbox = MboxCreate(1, MAX_MESSAGE);
    if(mbox < 0)
    {
        USLOSS_Console("spawnReal(): MboxCreate failed.\n");
    }
    int mboxStatus;

    // Convert the mboxID into a string
    char buf[MAX_MESSAGE];
    sprintf(buf, "%d", mbox);

    // Fork a new process
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnReal(): Calling fork1().\n");
    }
    int pid = fork1(name, spawnLaunch, buf, stackSize, priority);
    if (pid < 0)
    {
        mboxStatus = MboxRelease(mbox);
        if(mboxStatus == -1)
        {
            USLOSS_Console("spawnReal(): MboxRelease failed.\n");
        }
        return -1;
    }

    // Setup the proc table for the new proc
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnReal(): Writing information to process table for proc %d.\n", pid);
    }
    lock(ProcTableMutex);
    userProcPtr proc = &ProcTable[pid % MAXPROC];
    proc->pid = pid;
    proc->args = args;
    proc->startFunc = startFunc;
    proc->child = NULL;
    proc->nextSibling = NULL;
    // Add the new proc as a child of the current proc
    userProcPtr parent = &ProcTable[getpid() % MAXPROC];
    addChild(parent, proc);
    proc->parent = parent;
    unlock(ProcTableMutex);

    // Let spawnLaunch execute and release the mailbox
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnReal(): Coordinating with spawnLaunch for proc %d.\n", pid);
    }
    mboxStatus = MboxSend(mbox, NULL, 0);
    if(mboxStatus == -1)
    {
        USLOSS_Console("spawnReal(): MboxSend failed.\n");
    }

    // Return the pid of the new proc
    return pid;
}

/*
 * Launcher function for user mode procs. Reads the real starting function for
 * the process from the phase3 ProcTable.
 */
int spawnLaunch(char *arg)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnLaunch(): called\n");
    }
    // Unpack the mboxID from args
    int mboxID = atoi(arg);

    // Wait for spawnReal to finish setting up the proc table
    int mboxStatus = MboxReceive(mboxID, NULL, 0);
    if(mboxStatus == -1)
    {
        USLOSS_Console("spawnLaunch(): MboxReceive failed.\n");
    }

    MboxRelease(mboxID);
    if(mboxStatus == -1)
    {
        USLOSS_Console("spawnLaunch(): MboxRelease failed.\n");
    }

    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnLaunch(): Coordinating with spawnReal for proc %d.\n", getpid());
    }

    // Read information from the process table
    lock(ProcTableMutex);
    userProcPtr proc = &ProcTable[getpid() % MAXPROC];
    char *args = proc->args;
    int (*startFunc)(char *) = proc->startFunc;
    unlock(ProcTableMutex);

    // Check if we've been zapped
    if (isZapped())
    {
        terminateReal(0);
    }

    // Change the process into user mode
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("spawnLaunch(): Setting proc %d  to user mode.\n", getpid());
    }
    setToUserMode();

    // Call the start func and capture the return value
    int status = startFunc(args);

    // Terminate
    Terminate(status);
    return 0;
}

/*
 * Calls join to wait for a child to quit
 */
int waitReal(int *status)
{
    return join(status);
}

/*
 * Calls quit to terminate the running process.
 */
void terminateReal(int status)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("terminateReal(): called\n");
    }
    // Zap all the children of this process.
    lock(ProcTableMutex);
    userProcPtr thisProc = &ProcTable[getpid() % MAXPROC];
    userProcPtr childToZap = thisProc->child;
    unlock(ProcTableMutex);

    while (childToZap != NULL)
    {
        lock(ProcTableMutex);
        int pidToZap = childToZap->pid;
        unlock(ProcTableMutex);

        zap(pidToZap);

        lock(ProcTableMutex);
        childToZap = childToZap->nextSibling;
        unlock(ProcTableMutex);
    }

    // Remove this proc from the child list of our parent
    lock(ProcTableMutex);
    userProcPtr parent = thisProc->parent;
    removeChild(parent, thisProc);
    unlock(ProcTableMutex);

    // Quit with the desired status
    quit(status);
}

/*
 *  Create a semaphore with the given initSemValue. Called by semCreate
 */
int semCreateReal(int initSemValue)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semCreateReal(): called\n");
    }
    // Create the semaphore
    lock(semsMutex);
    int semHandle = getAvailableSemHandle();
    initSem(semHandle, initSemValue);
    currentNumSems++;
    unlock(semsMutex);

    return semHandle; // The handle is just the semID
}

/*
 *  Perform a P operation on the given semaphore. Called by semP
 */
void semPReal(int semHandle)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semPReal(): called\n");
    }
    semaphorePtr sem = &Semaphores[semHandle];
    // lock the mutex for this semaphore
    lock(sem->mutex);
    if(sem->count > 0)
    {
        sem->count--;
        // unlock mutex
        unlock(sem->mutex);
    }
    else
    {
        // unlock mutex
        unlock(sem->mutex);
        // Now block on the blockingMbox
        int status = MboxSend(sem->blockingMbox, NULL, 0);
        if(status == -1)
        {
            USLOSS_Console("semPReal(): error calling send on blockingMbox\n");
        }
    }
    if(sem->count == EMPTY)
    {
        if(DEBUG3 && debugflag3)
        {
            USLOSS_Console("semPReal(): semaphore was freed. Now terminating. \n");
        }
        // This semaphore was freed, so terminate
        terminateReal(1);
    }
}

/*
 *  Perform a V operation on the given semaphore. Called by semV
 */
void semVReal(int semHandle)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semVReal(): called\n");
    }
    semaphorePtr sem = &Semaphores[semHandle];
    // lock the mutex for this semaphore
    lock(sem->mutex);
    if(sem->count == 0)
    {
        int status = MboxCondReceive(sem->blockingMbox, NULL, 0);
        if(status == 1)
        {
            // There was no message to receive, so increment count
            sem->count++;
        }
    }
    else
    {
        sem->count++;
    }
    unlock(sem->mutex);
}

/*
 *  Free the given semaphore. Called by semFree
 */
int semFreeReal(int semHandle)
{
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("semFreeReal(): called\n");
    }
    lock(semsMutex);
    int returnStatus = freeSem(semHandle);
    currentNumSems--;
    unlock(semsMutex);

    return returnStatus;
}

/*
 *  Return the time of day, or the clock value, from USLOSS. Called by getTimeOfDay
 */
int getTimeOfDayReal()
{
    int timeOfDay = -1;

    int returnStatus = USLOSS_DeviceInput(USLOSS_CLOCK_INT, 0, &timeOfDay);

    if(returnStatus == USLOSS_DEV_INVALID)
    {
        USLOSS_Console("getTimeOfDayReal(): returned USLOSS_DEV_INVALID\n");
    }

    return timeOfDay;
}

/*
 *  Return the cpu time consumed by the current process. Called by cpuTime
 */
int cpuTimeReal()
{
    return readtime();
}

int getPIDReal()
{
    return getpid();
}
