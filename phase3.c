#include <usloss.h>
#include <usyscall.h>

#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include "sems.h"

// Debugging flag
int debugflag3 = 0;

// The sems table
semaphore Semaphores[MAXSEMS];

// The phase 3 proc table
userProc ProcTable[MAXPROCS];

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

    /*
     * Create first user-level process and wait for it to finish.
     * These are lower-case because they are not system calls;
     * system calls cannot be invoked from kernel mode.
     * Assumes kernel-mode versions of the system calls
     * with lower-case names.  I.e., Spawn is the user-mode function
     * called by the test cases; spawn is the kernel-mode function that
     * is called by the syscallHandler; spawnReal is the function that
     * contains the implementation and is called by spawn.
     *
     * Spawn() is in libuser.c.  It invokes USLOSS_Syscall()
     * The system call handler calls a function named spawn() -- note lower
     * case -- that extracts the arguments from the sysargs pointer, and
     * checks them for possible errors.  This function then calls spawnReal().
     *
     * Here, we only call spawnReal(), since we are already in kernel mode.
     *
     * spawnReal() will create the process by using a call to fork1 to
     * create a process executing the code in spawnLaunch().  spawnReal()
     * and spawnLaunch() then coordinate the completion of the phase 3
     * process table entries needed for the new process.  spawnReal() will
     * return to the original caller of Spawn, while spawnLaunch() will
     * begin executing the function passed to Spawn. spawnLaunch() will
     * need to switch to user-mode before allowing user code to execute.
     * spawnReal() will return to spawn(), which will put the return
     * values back into the sysargs pointer, switch to user-mode, and
     * return to the user code that called Spawn.
     */

    // spawn start 3
    int pid = spawnReal("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);

    // wait for start3 to finish
    int status;
    pid = waitReal(&status);
}

// TODO: Implement
void nullsys3(systemArgs *arg)
{
}

// TODO: Implement
void spawn(systemArgs *arg)
{
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
    int initSemValue = *((int*) args->arg1); // TODO: use val in pointer or pointer?
    int* args1Val = (void *) -1;
    if(initSemValue < 0)
    {
        args->arg1 = (void*) args1Val;
        args->arg4 = (void*) args1Val;
        return;
    }
    int handle = getAvailableHandle();  // TODO: implement
    if (handle == NO_SLOTS_AVAILABLE)
    {
        args->arg1 = (void*) args1Val;
        args->arg4 = (void*) args1Val;
        return;
    }
    *args1Val = handle;

    semaphore *sem;
    sem->count = initSemValue;
    Semaphores[handle] = sem;
    args->arg1 = (void*) args1Val;
    args->arg4 = (void *) 0;
    return;
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
int spawnReal(char *name, int (*startFunc)(char *), char *args, int stackSize, int priority)
{
    // Create a mailbox for synchronization with spawnLaunch
    int mbox = MboxCreate(0, MAX_MESSAGE);

    // Convert the mboxID into a string
    char buf[MAX_MESSAGE];
    sprintf(buf, "%d", mbox);

    // Fork a new process
    int pid = fork1(name, spawnLaunch, buf, stackSize, priority);

    // Setup the proc table for the new proc
    userProcPtr proc = &ProcTable[pid % MAXPROCS];
    proc->pid = pid;
    proc->args = args;
    proc->startFunc = startFunc;

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
    MboxReceive(mbox, NULL, 0);

    // Change the process into user mode
    setToUserMode();

    // Call the start func and capture return value
    userProcPtr proc = &ProcTable[getpid() % MAXPROCS];
    int status = proc->startFunc(proc->args);

    // Terminate
    terminateReal(status);
    return 0;
}

// TODO: Implement
void waitReal()
{
}

// TODO: Implement
void terminateReal()
{
}

// TODO: Implement
void semCreateReal()
{
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
