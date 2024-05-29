#include "taskimpl.h"

int taskcount;
int taskidgen;
int tasknswitch;

Task* taskrunning;
ucontext_t taskschedcontext;
Tasklist taskrunqueue;
Task** alltask;
int nalltask;

static void
contextswitch(ucontext_t* from, ucontext_t* to);
static void
assertstack(uint n);

static void
taskstart(uint x, uint y)
{
  Task* t;
  ulong z;

  z = ((ulong)x) << 32;
  z |= (ulong)y;
  t = (Task*)z;
  t->startfn(t->startarg);
  taskexit(t);
}

static Task*
taskalloc(void (*fn)(void*), void* arg, uint stk)
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
    .startfn = fn,
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
    alltask = realloc(alltask, (nalltask + 64) * sizeof(alltask[0]));
    if (alltask == 0) {
      exit(1);
    }
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
taskexit(Task* t)
{
}

void
taskswitch(void)
{
  assertstack(0);
  contextswitch(&taskrunning->uc, &taskschedcontext);
}

int
taskyield(void)
{
  int n;

  n = tasknswitch;
  taskswitch();
  return tasknswitch - n - 1;
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

static void
contextswitch(ucontext_t* from, ucontext_t* to)
{
  if (swapcontext(from, to)) {
    exit(1);
  }
}

static void
assertstack(uint n)
{
  Task* t;

  t = taskrunning;
  if ((uchar*)&t <= (uchar*)t->stk || (uchar*)&t - (uchar*)t->stk < 256 + n) {
    /* satck over flow */
    exit(1);
  }
}

#include <stdio.h>
void
run(void* x)
{
  int* y = x;
  printf("%d\n", y[0]);
}

int
main()
{
  int x = 10;
  taskcreate(run, &x, 10000);
}
