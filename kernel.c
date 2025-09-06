/*
  [03/06/25] game() -> entry()
  --downadow
 */


#include "keyboard.h"
#include "video.h"
#include "main.h"

void cmain (unsigned long magic, unsigned long addr)
{
  key_initialize();
  video_initialize();
  entry();
}

