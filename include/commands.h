#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_

#define UNIX_PATH_MAX 108
//int server_sock, client_sock;

struct single_command
{
  int argc;
  char** argv;
};

struct bg_info
{
  int pid;
  char cname[256];
  int flag;
} bg;


int evaluate_command(int n_commands, struct single_command (*commands)[512]);

int do_command(struct single_command * command);

void free_commands(int n_commands, struct single_command (*commands)[512]);

void print_hello(void * threadid);

void thread_todo(struct single_command * command);

void childhandler(int signalNo);

int do_exec(char * commands, char *** argv);
#endif // MYSH_COMMANDS_H_
