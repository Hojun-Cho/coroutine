/* Copyright (c) 2005 Russ Cox, MIT; see COPYRIGHT */

#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int quiet;
int goal;
int buffer = 0;

void
sender(void* arg)
{
  Channel* c;
  int p, i;
  c = arg;

  for (int i = 0; i < 10; i++) {
    printf("send %d %d\n", i, chansendul(c, i));
  }
}

void
reciver(void* arg)
{
  Channel* c;
  int p, i;
  c = arg;

  for (;;) {
    i = chanrecvul(c);
    printf("recv %d\n", i);
    if (i == 9)
      break;
  }
  taskexitall(0);
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
  printf("goal=%d\n", goal);
  c = newchan(sizeof(unsigned long), buffer);
  taskcreate(sender, c, 32768);
  taskcreate(reciver, c, 32768);
  for (;;) {
    taskyield();
  }
}
