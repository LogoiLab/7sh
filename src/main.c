#ifdef __linux__
#define _GNU_SOURCE //linux flag for execvpe.
#include <jemalloc/jemalloc.h> // a far superior allocator.
#endif
#include "common.h"
#include "builtin.h"

static size_t MAX_LEN = 2000;

int curr_proc;

void sigint_procs() { //easier than registering two signal handlers.
  kill(curr_proc, SIGINT);
}

void kill_shell() { //no longer used. If the user types exit then there is no currently running process so no need to kill those.
  sigint_procs();
  exit(EXIT_SUCCESS);
}

// BEGIN Borrowed from teenysh.c
char whitespace[] = " \t\r\n\v";

void cmd_not_found(char *cmd) {
  fprintf(stderr, "7sh: cmd not found: %s\n", cmd);
  exit(EXIT_FAILURE);
}
// END Borrowed from teenysh.c

void fork_run_wait(char *args[], int fpTo, int fpFrom) { //borrowed from teenysh.c pretty much in name only
  if ((curr_proc = fork()) == 0) { //store the fork pid so we can C-c it.
    if (fpTo) {
      dup2(fpTo, 1);
      dup2(fpTo, 2);
      close(fpTo); // close the fp
    }
    if (fpFrom) {
      dup2(fpFrom, 0);
      close(fpFrom);
    }
#ifdef __linux__ //allow use of path on systems that support it.
    char *path = getenv("PATH"); //tired of not having my path.
    char  pathenv[strlen(path) + sizeof("PATH=")]; //ensure the PATH= fits.
    char *envp[] = {pathenv, NULL}; //add a null.
    execvpe(args[0], args, envp); //supported on GNU+Linux
#endif
#ifndef __linux__
    execvp(args[0], args);//Nasty darwin
#endif
    cmd_not_found(args[0]);//if we fall through then the exec failed.
  }
  wait(&curr_proc);
}

void get_cmd(char * prompt, char **command) {
  if (strlen(prompt) > 0) {
    fprintf(stdout, "%s", prompt);//print custom prompt.
  } else {
    char username[MAX_LEN];
    char hostname[MAX_LEN];
    char cwd[MAX_LEN];
    getlogin_r(username, MAX_LEN - 2); // BEGIN From unistd docs.
    gethostname(hostname, MAX_LEN - 2);
    getcwd(cwd, MAX_LEN - 2); // END From unistd docs.
    fprintf(stdout, "[%s@%s] (%s) ⇶ ", username, hostname, cwd );//print the prompt
  }
  fflush(stdout); //not taking any chances here.
  fflush(stderr);
  fflush(stdin);
  size_t len = 0;
  getline(command, &len, stdin); //get the input
  strtok(*command, "\n"); //get rid of the newline
}

void process_cmd(char *cmd) {
  char **args = NULL;
  char * token = strtok(cmd, " "); //split off the first token
  int ct = 0;
  int isRedirString = 0;
  int outFP = 0;
  int inFP = 0;
  while (token != NULL) {
    if (token != NULL) {
      if (strcmp(token, ">") == 0) { //detect output redirect
        isRedirString = 1;
        outFP = 1;
      } else if (strcmp(token, "<") == 0) { //detect input redirect
        isRedirString = 1;
        inFP = 1;
      } else if (isRedirString) {
        isRedirString = 0;
        if (outFP) {
          outFP = open(token, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //prepare the file handle for writing
        }
        if (inFP) {
          inFP = open(token, O_RDONLY); //prepare the file handle for reading
        }
      } else {
        args = realloc(args, sizeof (char*) * (ct+1)); //clean up
        args[ct] = strdup(token); //store the token in the array
        ct++;
      }
      token = strtok(NULL, " "); //split off another token
    }
  }
  args = realloc(args, sizeof (char*) * (ct+1)); //make room for a null
  args[ct] = NULL; // add null terminator to keep execvp happy
  if(is_builtin(args[0]) >= 0){ // test for built in command (see builtin.c)
    exec_builtin(args); // hand off built in command (see builtin.c)
  } else {
    fork_run_wait(args, outFP, inFP); // fork and run the process
    free(args); // clean up to keep the stale pointers away
    args = NULL; // be extra sure those nasty stale pointers are gone
  }
}

int main (int argc, char **argv) {
  signal(SIGINT, sigint_procs); //Register C-c handler.
  signal(SIGTERM, kill_shell);  //Register C-d? handler?
  char * prompt = "";
  if (argc > 1) { // check if user supplied a prompt
    prompt = argv[1];
  }
  while(true) {
    char *cmd; // a place to store the raw command
    cmd = malloc(0);
    get_cmd(prompt, &cmd); // command retrieval function
    process_cmd(cmd); // process the command
    free(cmd); // clean up after ourselves
  }

  return EXIT_SUCCESS; //The shell shouldn't fail.
}
