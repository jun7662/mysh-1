#include "signal.h"
#include <signal.h>
#include <stdio.h>

void catch_sigint(int signalNo)
{
  // TODO: File this!
  signal(SIGINT,SIG_IGN);
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
  signal(SIGTSTP,SIG_IGN);
}

void test_func(){
  printf("test function runed!");
}

