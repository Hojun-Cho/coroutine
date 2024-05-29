#include "taskimpl.h"

int taskcount;
int taskidgen;

Tasklist taskrunqueue;
Task** alltask;
int nalltask;

static Task*
taskalloc(void (*taskstart)(void*), void* arg, uint stk)
{
  Task* t;
  sigset_t zero;
  uint x, y;
  ulong z;

  if ((t = malloc(sizeof *t + stk)) == nil) {
    exit(1);
  }
  *t = (Task){
    .stk = (uchar*)(&t[1]),
    .stksize = stk,
    .id = ++taskidgen,
    .startfn = taskstart,
    .startarg = arg,
    0,
  };
  sigemptyset(&zero);
  sigprocmask(SIG_BLOCK, &zero, &t->uc.uc_sigmask);
  if (getcontext(&t->uc)) {
    exit(1);
  }
  t->uc.uc_stack.ss_sp = t->stk;
  t->uc.uc_stack.ss_size = t->stksize;

  z = (ulong)t; /* ffffffff 00000000 */
  y = z;        /* y = 00000000 */
  x = z >> 32;  /* x = ffffffff */
  makecontext(&t->uc, (void (*)())taskstart, 2, y, x);
  return t;
}

int
taskcreate(void (*fn)(void*), void* arg, uint stk)
{
  int id;
  Task* t;

  t = taskalloc(fn, arg, stk);
  taskcount++;
  id = t->id;
  if (nalltask % 64 == 0) {
  }
  alltask[nalltask++] = t;
  taskready(t);
  return id;
}

void
taskready(Task* t)
{
  t->ready = 1;
  addtask(&taskrunqueue, t);
}

void
addtask(Tasklist* l, Task* t)
{
  if (l->tail) {
    l->tail->next = t;
    t->prev = l->tail;
  } else {
    l->head = t;
    t->prev = nil;
  }
  l->tail = t;
  t->next = nil;
}
