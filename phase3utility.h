#ifndef PHASE3UTLITY_H
#define PHASE3UTLITY_H

extern void genericSemaphoreInitialization();
extern int isSemAvailable();
extern int doesGivenSemExist(int);
extern int getAvailableSemHandle();
extern void initSem(int, int);
extern int freeSem(int);
extern void lock(int);
extern void unlock(int);
extern void setToUserMode();
#endif
