#include "taskimpl.h"

int taskcount;
int taskidgen;

static Task*
taskalloc(void (*fn)(void*), void* arg, uint stk)
{
  Task* t;

  if ((t = malloc(sizeof *t + stack)) == nil) {
    exit(1);
  }
  *t = { 0 };
  t->stk = (uchar*)(t + 1);
  t->stksize = stack;
  t->id = ++taskidgen;
  t->startfn = fn;
  t->startarg = arg;

	/* get context */

  return t;
}
