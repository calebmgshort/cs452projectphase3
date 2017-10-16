#ifndef PHASE3UTLITY_H
#define PHASE3UTLITY_H

extern int isSemAvailable();
extern int getAvailableSemHandle();
extern void initSem(int, int);
extern void lock(int);
extern void unlock(int);
#endif
