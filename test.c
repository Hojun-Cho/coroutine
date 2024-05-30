#include "taskimpl.h"
#include <stdio.h>

int var = 10;

static void
run(void* v)
{
  int* x;

  x = v;
  printf("%d\n", *x);
  taskexit(1);
}

void
taskmain(int argc, char** argv)
{
  printf("hihi\n");
  taskcreate(run, &var, 32768);
  taskyield();
}
