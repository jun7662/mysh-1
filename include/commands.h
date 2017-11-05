#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_

#define UNIX_PATH_MAX 108
struct single_command
{
  int argc;
  char** argv;
};

int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void free_commands(int n_commands, struct single_command (*commands)[512]);

#endif // MYSH_COMMANDS_H_
