#include "task.h"
#include <stdlib.h>
#include <ucontext.h>

#define nil ((void*)0)
#define nelem(x) (sizeof(x) / sizeof((x)[0]))

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long long uvlong;
typedef long long vlong;

enum
{
  STACK = 8192,
};

struct Task
{
  Task* next;
  Task* prev;
  Task* allnext;
  Task* allprev;
  ucontext_t context;
  uvlong alarmtime;
  uint id;
  uchar* stk;
  uint stksize;

  void (*startfn)(void*);
  void* startarg;
  void* udata;
};
