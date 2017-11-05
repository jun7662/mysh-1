#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "signal.h"
#include "commands.h"
#include "built_in.h"

#define SOCK_PATH "tpf_unix_sock.server"

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  char path[][40] = {"/usr/local/bin/", "/usr/bin/","/bin/", "/usr/sbin/","/sbin/"};
  for(int i =0; i < n_commands;i++) {
    struct single_command* com = (*commands+i);
    int pid;
    assert(com->argc != 0); 
    
    for(int j=0;j<com->argc;j++){
      if(strstr(com->argv[j],"~") != NULL){
	char home[] = "/home/aeis/";
        char * tmp = (char*)malloc(strlen(com->argv[j])+1);
	tmp = strstr(com->argv[j],"~")+1;
	com->argv[j] = (char*)malloc(strlen(com->argv[j])+12);
        strcpy(com->argv[j], "/home/aeis/");
	strcat(com->argv[j],tmp);
      }
    }
    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } else {
	pid = fork();
        if(pid==0){
      /*for(int j = 0; j<com->argc;j++){
        if(strcmp(com->argv[j],"&")==0){
          free(com->argv[j]);
          com->argc -= 1;
          if(fork()==0)
        return 0;
          execv(com->argv[0],com->argv);
      }
    }*/
      execv(com->argv[0],com->argv);
      char * tmp = (char*)malloc(strlen(com->argv[0])+1);
      strcpy(tmp,com->argv[0]);
      com->argv[0] = (char*)malloc(strlen(path[0])+strlen(com->argv[0]) + 1);
      com->argv[0] = strcat(path[0],tmp);
      execv(com->argv[0],com->argv);
      
      for(int j = 1;j<5;j++){
        com->argv[0] = (char*)malloc(strlen(com->argv[0])-strlen(path[j-1])+strlen(path[j])+1);
        com->argv[0] = strcat(path[j],tmp);
        execv(com->argv[0],com->argv);
    }
      com->argv[0] = (char*)malloc(strlen(com->argv[0])-5);
      strcpy(com->argv[0],tmp);
      fprintf(stderr, "%s: command not found\n",com->argv[0]);
      return 1;
      }
    printf("done\n");
    wait(&pid);
    }
  }
  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
