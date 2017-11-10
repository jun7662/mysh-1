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
  pthread_t thread;
  char buf[256];
  int rc;
  long t = 2;
  int client_sock, server_sock;
  int sock[2];
  int len, oldstdout, oldstdin, oldstderr;
  struct sockaddr_un s_addr,  c_addr;

  memset(&s_addr, 0, sizeof(struct sockaddr_un));
  memset(&c_addr, 0, sizeof(struct sockaddr_un));
  memset(buf, 0, 256);
  
  if(n_commands > 1){
    pthread_create(&thread, NULL, thread_todo, (void *)server_sock);
    
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(client_sock == -1){
      fprintf(stderr,"CLIENT SOCKET ERROR\n");
      return 0;
    }
    c_addr.sun_family = AF_UNIX;
    strcpy(c_addr.sun_path, CLIENT_PATH);
    /*unlink(CLIENT_PATH);
    if(bind(client_sock, (struct sockaddr *) &c_addr, sizeof(c_addr))){
      fprintf(stderr, "CLIENT BIND ERROR\n");
      close(client_sock);
      return 0;
    }*/
    //c_addr.sun_family = AF_UNIX;
    //strcpy(c_addr.sun_path,CLIENT_PATH);
    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, SERVER_PATH);
    while(1) if(connect (client_sock, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) break;
    close(1);
    dup2(client_sock,1);
    close(client_sock);
  }
  
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
      } // path resoultion ~
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
	  /*if(n_commands > 1){
	    pthread_create(&thread, NULL, thread_todo, (void *)server_sock);
   	    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
 	    if(client_sock == -1){
	      fprintf(stdout,"CLIENT SOCKET ERROR");
	      return 0;
	    }
	  s_addr.sun_family = AF_UNIX;
	  strcpy(s_addr.sun_path, SERVER_PATH);
	  while(1) if(connect (client_sock, (struct sockaddr*) &s_addr, sizeof(s_addr)) == -1) break;
	  close(1);
	  dup2(client_sock, 1);
          close(client_sock);
	}*/
	  //dup2(client_sock,1);
	  //close(client_sock);
          do_exec(com->argv[0], com->argv);
          fprintf(stderr, "%s: command not found\n",com->argv[0]);
          return 1;
    }
      //if(n_commands > 1){
        //pthread_join(thread, NULL);
	//dup2(client_sock,0);
        //printf("done\n");
        //close(client_sock);
      //}
      wait(&pid);
      printf("done\n");
    //recv(client_sock, buf, sizeof(buf), 0);
    //if (read(client_sock, buf, sizeof(buf))>0);
      //close(client_sock);
    //fprintf(stdout, "buf : %s\n", buf);
    //printf("buf : %s\n",buf);
    //else printf("asd\n");
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
  int server_sock, len, client_sock;
  struct sockaddr_un server_addr, client_addr;

  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if(server_sock == -1) {
    fprintf(stderr, " in thread : SERVER SOCKET ERROR\n");
    exit(1);
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, SERVER_PATH);
  client_addr.sun_family = AF_UNIX;
  strcpy(client_addr.sun_path, CLIENT_PATH);
  len = sizeof(client_addr);
  unlink(SERVER_PATH);
  if ( bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    fprintf(stderr, "in thread : SERVER BIND ERROR\n");
    close(server_sock);
    exit(1);
  }

  while(1){
    if ( listen(server_sock, 5) == -1){
      fprintf(stderr,"in thread : LISTEN ERROR\n");
      close(server_sock);
      exit(1);
    }
    client_sock = accept(server_sock, (struct sockaddr *) &client_sock, &len);
    if (client_sock == -1){
      fprintf(stderr,"in thread : ACCPET ERROR\n");
      close(server_sock);
      exit(1);
    }else break;
//    if(fork()==0){
//      close(server_sock);
//      dup2(client_sock, 0);
//      close(client_sock);
//      exit(0); 
//    }
//    wait(0);
  }
  close(0);
  dup2(client_sock,0);
    close(client_sock);
    close(server_sock);
  pthread_exit(NULL);
}

int do_exec(char * commands, char ** argv){
  char path[][40] = {"/usr/local/bin/", "/usr/bin/","/bin/", "/usr/sbin/","/sbin/"};

  execv(argv[0],argv);
  char * tmp = (char*)malloc(strlen(argv[0])+1);
  strcpy(tmp,argv[0]);
  argv[0] = (char*)malloc(strlen(path[0])+strlen(argv[0]) + 1);
  argv[0] = strcat(path[0],tmp);
  execv(argv[0],argv);
 
  for(int j = 1;j<5;j++){
    argv[0] = (char*)malloc(strlen(argv[0])-strlen(path[j-1])+strlen(path[j])+1);
    argv[0] = strcat(path[j],tmp);
    execv(argv[0],argv);
  }
  argv[0] = (char*)malloc(strlen(argv[0])-5);
  strcpy(argv[0],tmp);
}
