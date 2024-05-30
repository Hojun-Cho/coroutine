#include "taskimpl.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

Rendez* r;
QLock* l;
int var = 10;

static void
waketask(void* v)
{
  printf("try wake up all \n");
  taskwakeupall(r);
  printf("done \n");
  taskexit(1);
}
static void
sleeptask(void* v)
{
  int* x;

  x = v;
  printf("%d\n", *x);
  qlock(l);
  tasksleep(r);
  printf("wake up !!\n");
  taskexit(1);
}

void
taskmain(int argc, char** argv)
{
  printf("print 4 time\n");
  l = newqlock();
  qlock(l);
  r = newrendez(l);
  taskcreate(sleeptask, &var, 4096);
  taskcreate(sleeptask, &var, 4096);
  taskcreate(sleeptask, &var, 4096);
  taskcreate(sleeptask, &var, 4096);
  taskcreate(waketask, 0, 4096);
  tasksleep(r);
}
