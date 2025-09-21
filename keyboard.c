/*
  [03/06/25] key_decode() changed
  [03/18/25] *key='\0'
  --downadow
 */

#include "keyboard.h"

#include "io.h"
#include "memory.h"

static uint8 *ringbuf=0;
static uint32 ringstart=0, ringend=0, ringsize=1024;

void key_decode(uint8 *key, bool *pressed)
{
  uint8 c;
  uint32 ringoldstart = ringstart;

  *key='\0';
  *pressed=false;
    
  if (ringstart == ringend) return;
  c=ringbuf[ringstart++];
  if (ringstart==ringsize) ringstart=0;

  if (c == 0xe0) {
    if (ringstart == ringend) {
      ringstart = ringoldstart;
      return;
    };
    c = ringbuf[ringstart++];
    if (ringstart == ringsize) ringstart=0;
    *pressed = ((c&0x80) == 0) ? true : false;
    c &= ~0x80;
    *key = c;
  }else{
    *pressed = ((c&0x80) == 0) ? true : false;
    c &= ~0x80;
    *key = c;
  };
};

void key_polling()
{
  if (inb(0x64)&1) {
    ringbuf[ringend++] = inb(0x60);
    if (ringend==ringsize) ringend=0;
  };
};

void key_initialize()
{
  ringbuf = malloc (ringsize);
};
