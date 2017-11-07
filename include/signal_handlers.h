#ifndef MYSH_SIGNAL_H_
#define MYSH_SIGNAL_H_

#include <stdlib.h>

void catch_sigint(int signalNo);

void catch_sigtstp(int signalNo);
#endif // MYSH_SIGNAL_H_
