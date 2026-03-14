// MemoryFree library based on code posted here:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
// Extended by Matthew Murdoch to include walking of the free list.

#ifndef	MEMORY_FREE_H
#define MEMORY_FREE_H

#ifdef __cplusplus
extern "C" {
#endif

int freeMemory();

#ifdef  __cplusplus
}
#endif

// Inline implementation (Arduino IDE does not compile .cpp in subdirectories)
#if defined(__arm__) && defined(TEENSYDUINO)
// Teensy 4.x (ARM Cortex-M7)
extern char *__brkval;

inline int freeMemory()
{
  char top;
  return &top - __brkval;
}

#else
// AVR
extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist
{
  size_t sz;
  struct __freelist *nx;
};

extern struct __freelist *__flp;

inline int freeListSize()
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

inline int freeMemory()
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

#endif
