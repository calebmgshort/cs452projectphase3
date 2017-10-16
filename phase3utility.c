/*
 *  Determines if there is an available slot in the sem table
 *  Returns: 1 if yes, 0 if no
 */
int isSemAvailable()
{
    return -1;
}

/*
 *  Gets the first available semaphore handle and returns it
 *  Returns -1 if no availabe handle exists
 */
int getAvailableSemHandle()
{
    return -1;
}

/*
 *  Initializes the semaphore at semHandle with the given initSemValue
 */
void initSem(int semHandle, int initSemValue)
{

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

void setToUserMode()
{
    // TODO
}
