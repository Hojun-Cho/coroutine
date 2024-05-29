#ifndef _TASK_H_
#define _TASK_H_

typedef struct Task Task;
typedef struct Tasklist Tasklist;

struct Tasklist
{
  Task* head;
  Task* tail;
};

typedef struct QLock
{
  Task* owner;
  Tasklist* wwaiting;
} QLock;

void
qlock(QLock*);
void
canqlock(QLock*);
void
qunlock(QLock*);

#endif
