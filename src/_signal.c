#include <stdio.h>
#include "_signal.h"


void catch_sigint(int signalNo)
{
  // TODO: File this!
  printf("\n");
  return;
}

void catch_sigtstp(int signalNo)
{
  printf("\n");
  return;
  // TODO: File this!
}
