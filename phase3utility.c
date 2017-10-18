#include <usyscall.h>
#include <stdlib.h>
#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include "sems.h"

/*
 *  A file of utility functions
 */

extern semaphore Semaphores[MAXSEMS];
extern int currentNumSems;
extern int semsMutex;
extern int debugflag3;

/*
 *  Initialize all semaphore stuff
 */
void genericSemaphoreInitialization()
{
    // Initialize the semaphore table
    int i;
    for(i = 0; i < MAXSEMS; i++)
    {
        Semaphores[i].count = EMPTY;
        Semaphores[i].mutex = EMPTY;
        Semaphores[i].blockingMbox = EMPTY;
    }
    semsMutex = MboxCreate(1, 0);
    if(semsMutex < 0)
    {
        USLOSS_Console("genericSemaphoreInitialization(): Error creating mutex for semaphores\n");
    }
}

/*
 *  Determines if there is an available slot in the sem table
 *  Returns: 1 if yes, 0 if no
 */
int isSemAvailable()
{
    if(currentNumSems == MAXSEMS)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
 *  Gets the first available semaphore handle and returns it
 *  Returns -1 if no available handle exists
 */
int getAvailableSemHandle()
{
    // Return an empty slot in the sem table if one exists
    for(int i = 0; i < MAXSEMS; i++)
    {
        if(Semaphores[i].count == EMPTY)
        {
            if(DEBUG3 && debugflag3)
            {
                USLOSS_Console("getAvailableSemHandle(): found sem slot %d\n", i);
            }
            return i;
        }
    }
    // Otherwise return the invalid value
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("getAvailableSemHandle(): no slots available\n");
    }
    return NO_SLOTS_AVAILABLE;
}

/*
 *  Initializes the semaphore at semHandle with the given initSemValue
 */
void initSem(int semHandle, int initSemValue)
{
    semaphorePtr sem = &Semaphores[semHandle];
    sem->count = initSemValue;
    int mutex = MboxCreate(1, 0);
    if(mutex < 0)
    {
        USLOSS_Console("genericSemaphoreInitialization(): Error creating mutex for sem\n");
    }
    sem->mutex = mutex;
    int blockingMbox = MboxCreate(0, 0);
    if(blockingMbox < 0)
    {
        USLOSS_Console("genericSemaphoreInitialization(): Error creating blockingMbox for sem\n");
    }
    sem->blockingMbox = blockingMbox;
}

/*
 *  Free the semaphore with the given handle
 *  Returns 1 if 1 or more processes were blocked on this mailbox; 0 otherwise
 */
int freeSem(int semHandle)
{
    semaphorePtr sem = &Semaphores[semHandle];
    sem->count = EMPTY;

    // Free the mailboxes associated with this semaphore
    int status = MboxRelease(sem->mutex);
    if(status == -1)
    {
        USLOSS_Console("freeSem(): Error releasing mutex\n");
    }
    sem->mutex = EMPTY;

    // Do a CondReceive to determine if there is at least 1 blocked proc
    int notReturnStatus = MboxCondReceive(sem->blockingMbox, NULL, 0);
    if(notReturnStatus == -1)
    {
        USLOSS_Console("freeSem(): Error getting blocked proc\n");
    }
    status = MboxRelease(sem->blockingMbox);
    if(status == -1)
    {
        USLOSS_Console("freeSem(): Error releasing blockingMbox\n");
    }
    sem->blockingMbox = EMPTY;

    if(notReturnStatus == -2)
    {
        return 0;
    }
    else if(notReturnStatus >= 0)
    {
        return 1;
    }
    else if(notReturnStatus == -1)
    {
        return -1;
    }
    else
    {
        return -5;
    }
}

/*
 *  Determines if a semaphore with the given handle exists
 *  Returns 0 if no. Returns 1 if yes
 */
int doesGivenSemExist(int semHandle)
{
    if(semHandle < 0 || semHandle > MAXSEMS)
    {
        return 0;
    }
    else if(Semaphores[semHandle].count == EMPTY)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
 * Acquires the lock for the given mutex mailbox.
 */
void lock(int mutexMbox)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("lock(): Acquiring lock for mutex mailbox %d\n", mutexMbox);
    }
    int result = MboxSend(mutexMbox, NULL, 0);
    if(result == -1)
    {
        USLOSS_Console("lock(): error calling send on mutex\n");
    }
    if (result == -3)
    {
        // TODO what to do if we've been zapped?
    }
}

/*
 * Releases the lock for the given mutex mailbox.
 */
void unlock(int mutexMbox)
{
    if (DEBUG3 && debugflag3)
    {
        USLOSS_Console("unlock(): Releasing the lock on mutex mailbox %d\n", mutexMbox);
    }
    int result = MboxReceive(mutexMbox, NULL, 0);
    if(result == -1)
    {
        USLOSS_Console("unlock(): error calling receive on mutex\n");
    }
    if (result == -3)
    {
        // TODO what to do if we've been zapped?
    }
}

/*
 * Sets the current process into user mode. Requires the process to currently
 * be in kernel mode. Also enables interrupts.
 */
void setToUserMode()
{
    unsigned int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE))
    {
        USLOSS_Console("setToUserMode(): Called from user mode.  Halting...\n");
        USLOSS_Halt(1);
    }
    unsigned int newpsr = (psr & ~USLOSS_PSR_CURRENT_MODE) | USLOSS_PSR_CURRENT_INT;
    int result = USLOSS_PsrSet(newpsr);
    if (result != USLOSS_DEV_OK)
    {
        USLOSS_Console("setToUserMode(): Bug in psr set.  Halting...\n");
        USLOSS_Halt(1);
    }
}

/*
 * Adds child to the child list of parent. Parent cannot be NULL.
 */
void addChild(userProcPtr parent, userProcPtr child)
{
    if (parent->child == NULL)
    {
        parent->child = child;
    }
    else
    {
        userProcPtr olderSibling = parent->child;
        while (olderSibling->nextSibling != NULL)
        {
            olderSibling = olderSibling->nextSibling;
        }
        olderSibling->nextSibling = child;
    }
}

/*
 * Removes up to one proc that == child from the child list of parent.
 * Parent cannot be NULL.
 */
void removeChild(userProcPtr parent, userProcPtr child)
{
    if (parent->child == child)
    {
        parent->child = parent->child->nextSibling;
        return;
    }
    userProcPtr olderSibling = parent->child;
    while (olderSibling->nextSibling != NULL)
    {
        if (olderSibling->nextSibling == child)
        {
            olderSibling->nextSibling = olderSibling->nextSibling->nextSibling;
            return;
        }
        olderSibling = olderSibling->nextSibling;
    }
}
