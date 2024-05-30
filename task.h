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
  Tasklist waiting;
} QLock;

void
taskmain(int argc, char** argv);
int
taskyield(void);

void
qlock(QLock*);
int
canqlock(QLock*);
void
qunlock(QLock*);
QLock*
newqlock();

typedef struct Rendez Rendez;
struct Rendez
{
  QLock* l;
  Tasklist waiting;
};

void
tasksleep(Rendez*);
int
taskwakeup(Rendez*);
int
taskwakeupall(Rendez*);
Rendez*
newrendez(QLock* l);

extern Task* taskrunning;
extern int taskcount;

#endif
