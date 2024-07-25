#include "task.h"
#include "print.h"
#include <stdlib.h>
#include <unistd.h>

int quiet;
int goal;
int buffer = 10;

void
primetask()
{
  Channel *c, *nc;
  int p, i;

	c = taskarg();
  p = chanrecvul(c);
  if (p > goal)
    taskexitall(0);
  if (!quiet)
    print("%d\n", p);
  nc = newchan(sizeof(unsigned long), buffer);
  taskcreate(primetask, nc, 32768);
  for (;;) {
    i = chanrecvul(c);
    if (i % p)
      chansendul(nc, i);
  }
}

void
taskmain(int argc, char** argv)
{
  int i;
  Channel* c;

  if (argc > 1)
    goal = atoi(argv[1]);
  else
    goal = 100;
  print("goal = %d\n", goal);
  c = newchan(sizeof(unsigned long), buffer);
  taskcreate(primetask, c, 32768);
  for (i = 2;; i++) {
    chansendul(c, i);
  }
}

void*
emalloc(unsigned long n)
{
  return calloc(n, 1);
}

long
lrand(void)
{
  return rand();
}
