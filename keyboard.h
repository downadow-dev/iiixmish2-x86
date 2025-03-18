/* [03/18/25] remove comment;     --downadow */

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "types.h"

void key_decode(uint8 *key, bool *pressed);
void key_polling();
void key_initialize();

#endif
