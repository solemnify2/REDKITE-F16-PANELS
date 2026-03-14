#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "MemoryFree.h"

#if defined(__arm__) && defined(TEENSYDUINO)
// Teensy 4.x (ARM Cortex-M7) implementation
extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

int freeMemory()
{
  char top;
  return &top - __brkval;
}

#else
// AVR implementation
extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist
{
  size_t sz;
  struct __freelist *nx;
};

extern struct __freelist *__flp;

int freeListSize()
{
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx)
  {
    total += 2;
    total += (int) current->sz;
  }
  return total;
}

int freeMemory()
{
  int free_memory;
  if ((int)__brkval == 0)
  {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  }
  else
  {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}
#endif
