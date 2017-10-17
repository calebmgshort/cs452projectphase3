#include <stddef.h>
#include <usloss.h>
#include "sems.h"

extern int debugflag3;

void
p1_fork(int pid)
{
    if (DEBUG3 && debugflag3);
//        USLOSS_Console("p1_fork() called: pid = %d\n", pid);
} /* p1_fork */

void
p1_switch(int old, int new)
{
    if (DEBUG3 && debugflag3);
//        USLOSS_Console("p1_switch() called: old = %d, new = %d\n", old, new);
} /* p1_switch */

void
p1_quit(int pid)
{
    if (DEBUG3 && debugflag3);
//        USLOSS_Console("p1_quit() called: pid = %d\n", pid);
} /* p1_quit */
