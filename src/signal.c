#include "signal.h"
#include <signal.h>

void catch_sigint(int num)
{
  // TODO: File this!
  signal(SIGINT,SIG_IGN);
}

void catch_sigtstp(int num)
{
  // TODO: File this!
  signal(SIGTSTP,SIG_IGN);
}
