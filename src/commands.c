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
#include <pthread.h>

#include "signal.h"
#include "commands.h"
#include "built_in.h"

#define SERVER_PATH "my_socket.server"
#define CLIENT_PATH "my_socket.client"

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
  pthread_t thread;
  char buf[256];
  int rc;
  long t = 2;
  int client_sock, server_sock;
  int sock[2];
  int len;
  struct sockaddr_un s_addr,  c_addr;

  memset(&s_addr, 0, sizeof(struct sockaddr_un));
  memset(&c_addr, 0, sizeof(struct sockaddr_un));
  memset(buf, 0, 256);

  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_sock == -1){
    printf("Socket Error\n");
    return 0;
  }
  s_addr.sun_family = AF_UNIX;
  strcpy(s_addr.sun_path, SERVER_PATH);
  len = sizeof(s_addr);
  unlink(SERVER_PATH);
  rc = bind(server_sock, (struct sockaddr *) &s_addr, len);
  if (rc == -1){
    printf("Bind Error\n");
    close(server_sock);
    return 0;
  }

  client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_sock == -1) {
    printf("CSE\n");
    return 0;
  }
  c_addr.sun_family = AF_UNIX;
  strcpy(c_addr.sun_path, CLIENT_PATH);
  unlink(CLIENT_PATH);
  if(bind(client_sock, (struct sockaddr *)&c_addr, len) == -1){
    printf("CBE\n");
    return 0;
  }
  //rc = pthread_create(&thread, NULL ,thread_todo, (void *) server_sock);
  rc = listen(server_sock, 10);  
  if (rc ==-1) {
    printf("Listen Error\n");
    return 0;
  }
  rc = connect(client_sock, (struct sockaddr *)&s_addr,sizeof(s_addr));
  client_sock = accept(server_sock, (struct sockaddr *)&c_addr,&len);
  if (client_sock == -1){
    printf("Accpet Error\n");
    close(server_sock);
    close(client_sock);
    return 0;
  }
  
  printf("socket connected\n");
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
      printf("before dup\n");
      dup2(client_sock, STDIN_FILENO);
      dup2(client_sock, STDOUT_FILENO);
      dup2(client_sock, STDERR_FILENO);
      close(client_sock);
      printf("dep ok\n");
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
    close(client_sock);
    wait(&pid);
    printf("done\n");
    recv(client_sock, buf, sizeof(buf), 0);
    //if (read(client_sock, buf, sizeof(buf))>0)
      printf("buf : %s\n",buf);
    //else printf("asd");
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

void print_hello(void* threadid){
  long tid;
  tid = (long)threadid;
  printf("%ld\n", tid);  
}

void thread_todo (void * sock){
  
}
