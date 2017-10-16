/*
 *  A file of utility functions
 */

extern semaphore Semaphores[MAXSEMS];
extern int currentNumSems;

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
        Semaphores[i].firstBlockedProc = NULL;
    }
    semsMbox = MboxCreate(1, 0);
    if(semsMbox < 0)
    {
        USLOSS_Console("genericSemaphoreInitialization(): Error creating mutex for semaphores");
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
    int i;
    for(int i = 0; i < MAXSEMS; i++)
    {
        if(Semaphores[i].count != EMPTY)
        {
            if(DEBUG3 && debugflag3)
            {
                USLOSS_Console("getAvailableSemHandle(): found sem slot %d", i);
            }
            return i;
        }
    }
    // Otherwise return the invalid value
    if(DEBUG3 && debugflag3)
    {
        USLOSS_Console("getAvailableSemHandle(): no slots available");
    }
    return NO_SLOTS_AVAILABLE;
}

/*
 *  Initializes the semaphore at semHandle with the given initSemValue
 */
void initSem(int semHandle, int initSemValue)
{
    semaphore sem = Semaphores[semHandle];
    sem.count = initSemValue;
    sem.firstBlockedProc = NULL;
}

/*
 *  Free the semaphore with the given handle
 */
void freeSem(int semHandle)
{
    semaphore sem = Semaphores[semHandle];
    sem.count = EMPTY;
    // For each blocked proc, take it off this list, (zap it?) and put it back on the ready list
    if(sem.firstBlockedProc != NULL)
    {
        userProcPtr current = sem.firstBlockedProc;
        userProcPtr next = NULL;

        while(current != NULL){
          next = current->nextBlockedProc;
          current->nextBlockedProc = NULL;
          zap(current.pid);
          // TODO: Put this proc back on the ready list how?
          current = next;
        }
        sem.firstBlockedProc = NULL;
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
    int result = MboxSend(mutexMbox, NULL, 0);
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
    int result = MboxReceive(mutexMbox, NULL, 0);
    if (result == -3)
    {
        // TODO what to do if we've been zapped?
    }
}
