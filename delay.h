/*
  [03/07/25] changes for iiixmish2-x86
  --downadow
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "types.h"

/* [03/06/25] added SEC    --downadow */
#define SEC   1193180

// initialize the keyboard before using delay_wait !!!

// t in seconds.
// x = 1193180 / t
void delay_wait (uint32 x);

void timer_reset(void);
uint16 timer_read(void);

#endif
