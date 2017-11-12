#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_

#define UNIX_PATH_MAX 108
//int server_sock, client_sock;

struct single_command
{
  int argc;
  char** argv;
};

int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void free_commands(int n_commands, struct single_command (*commands)[512]);

void print_hello(void * threadid);

void thread_todo(void * sock);

int do_exec(char * commands, char *** argv);
#endif // MYSH_COMMANDS_H_
