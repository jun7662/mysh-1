#include <stdio.h>
#include "signal_handlers.h"


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
