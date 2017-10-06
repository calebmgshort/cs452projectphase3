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

pid = spawnReal("start3", start3, NULL, USLOSS_MIN_STACK, 3);

/* Call the waitReal version of your wait code here.
 * You call waitReal (rather than Wait) because start2 is running
 * in kernel (not user) mode.
 */

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


// TODO: Implement
void spawnReal()
{
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
