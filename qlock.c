#include "taskimpl.h"

void
_qlock(Qlock* l, int block)
{
  if (l->owner == nil) {
    l->owner = taskrunning;
    return 1;
  }
  if (!block)
    return 0;
  addtask(&l->waiting, taskrunning);
  taskswitch(); /* wait until own lock */
  if (l->owner != taskrunning) {
    /* TODO: */
    exit(1);
  }
  return 1;
}

void
qlock(QLokc* l)
{
  _qlock(l, 1);
}

int
canqlock(QLock* l)
{
  return _qlock(l, 0);
}

void
qunlock(QLock* l)
{
  Task* ready;

  if (l->owner == 0) {
    /* TODO: */
    exit(1);
  }
  l->owner = ready = l->wwaiting.head;
  if (l->owner != nil) {
    deltask(&l->wwaiting, ready);
    taskready(ready);
  }
}
