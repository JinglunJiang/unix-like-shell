#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include "job.h"
#include "shell.h"
#include "history.h"
#include "signal_handlers.h"

volatile pid_t foreground_pid = -1;
msh_t *shell;

const int MAX_LINE = 1024;
const int MAX_JOBS = 16;
const int MAX_HISTORY = 10;

msh_t *alloc_shell(int max_jobs, int max_line, int max_history)
{

  msh_t *shell = malloc(sizeof(msh_t));

  shell->max_jobs = (max_jobs != 0) ? max_jobs : MAX_JOBS;
  shell->max_line = (max_line != 0) ? max_line : MAX_LINE;
  shell->max_history = (max_history != 0) ? max_history : MAX_HISTORY;

  // shell->jobs = malloc(shell->max_jobs * sizeof(job_t));
  // for (int i = 0; i < shell->max_jobs; i++) {
  //   shell->jobs[i].state = UNDEFINED;
  // }

  shell->jobs = alloc_jobs(shell->max_jobs);

  shell->history = alloc_history(shell->max_history);

  shell->foreground_pid = -1;

  initialize_signal_handlers(shell); 

  return shell;
}

static char *internal_ptr = NULL;

char *parse_tok(char *line, int *job_type) {
  if (line != NULL) {
    internal_ptr = line;
  }

  if (*internal_ptr == '\0') {
    *job_type = -1;
    return NULL;
  }

  char *job = internal_ptr; // Keep track of the place after the last call
  bool met_separator = false;
  char *last_non_space = NULL;

  while (*internal_ptr != '\0') {
    if (*internal_ptr == '&') {
      *job_type = 0;
      *internal_ptr = '\0';
      met_separator = true;
      internal_ptr++;
      return job;
    }
    if (*internal_ptr == ';') {
      *job_type = 1;
      *internal_ptr = '\0';
      met_separator = true;
      internal_ptr++;
      return job;
    }
    if (*internal_ptr != ' ') {
      last_non_space = internal_ptr; // Update the last non-space position
    }
    internal_ptr++;
  }

  if (*internal_ptr == '\0') {
    if (last_non_space != NULL) {
      *job_type = 1;
      internal_ptr = last_non_space + 1; 
      return job;
    } else {
      *job_type = -1;
      return NULL;
    }
  }

  if (!met_separator) {
    *job_type = 1;
  }
  return job;
}

char **separate_args(char *line, int *argc, bool *is_builtin)
{

  *argc = 0;

  if (line == NULL || line[0] == '\0')
  {
    return NULL;
  }

  void compute_num_args()
  { // helper function used to pre-calculated the number of arguments
    int i = 0;
    int inArgument = 0; // keep track whether the pointer is currently inside an argument
    while (line[i] != '\0')
    {
      if (line[i] == ' ')
      {
        inArgument = 0;
      }
      else
      {
        if (inArgument == 0)
        {
          (*argc)++;
        }
        inArgument = 1;
      }
      i++;
    }
  }

  compute_num_args(); // calculate the number of arguments

  char **argv = malloc((*argc + 1) * sizeof(char *)); // allocate memory for the number of arguments
  char *ptr = line;
  int argIndex = 0;

  while (*ptr != '\0')
  {
    while (*ptr == ' ')
    { // ignoring extra spaces
      ptr++;
    }
    char *argStart = ptr; // the start of current argument
    while (*ptr != ' ' && *ptr != '\0')
    {
      ptr++;
    }
    int argLength = ptr - argStart;
    argv[argIndex] = malloc(sizeof(char) * (argLength + 1)); // allocate memory for each argument
    strncpy(argv[argIndex], argStart, argLength);            // copy the argument into the array
    argv[argIndex][argLength] = '\0';                        // end of an argument
    argIndex++;                                              // increment index to the next item in the array
  }

  argv[*argc] = NULL; // the last item in the array assigned to NULL

  if (*argc > 0){
    if (strcmp(argv[0], "jobs") == 0 ||
        strcmp(argv[0], "history") == 0 ||
        (argv[0][0] == '!' && strlen(argv[0]) > 1) ||
        strcmp(argv[0], "bg") == 0 ||
        strcmp(argv[0], "fg") == 0 ||
        strcmp(argv[0], "kill") == 0){
      *is_builtin = true;
    }
    else{
      *is_builtin = false;
    }
  }
  else{
    *is_builtin = false;
  }

  return argv;
}

void waitfg(pid_t pid)
{ 
  while (1){
    if (pid != foreground_pid){
      break;
    }
    sleep(1);
  }
}

char *builtin_cmd(msh_t *shell, char **argv){

  if (strcmp(argv[0], "jobs") == 0){

    for (int i = 0; i < shell->max_jobs; i++){
      if (shell->jobs[i].state == BACKGROUND || shell->jobs[i].state == SUSPENDED){
        int jid = shell->jobs[i].jid;
        int pid = shell->jobs[i].pid;
        char *cmd_line = shell->jobs[i].cmd_line;
        char *state = (shell->jobs[i].state == BACKGROUND) ? "RUNNING" : "Stopped";
        printf("[%d] %d %s %s\n", jid, pid, state, cmd_line);
      }
    }

    return NULL;
  }
  else if (strcmp(argv[0], "history") == 0){

    print_history(shell->history);
    return NULL;

  }
  else if (argv[0][0] == '!' && strlen(argv[0]) > 1){

    int isNumeric = 1;
    for (size_t i = 1; i < strlen(argv[0]); i++){
      if (!isdigit(argv[0][i])){
        isNumeric = 0;
        break;
      }
    }

    if (isNumeric){
      int history_index = atoi(&argv[0][1]);
      char *chosen_command = find_line_history(shell->history, history_index);
      evaluate(shell, chosen_command); 
      printf("%s\n", chosen_command);
    }

    return NULL;
  }
  else if (strcmp(argv[0], "bg") == 0 || strcmp(argv[0], "fg") == 0){

    job_t *job = NULL;

    if (argv[1] != NULL){
      if (argv[1][0] == '%'){
        int job_id = atoi(&argv[1][1]);
        for (int i = 0; i < shell->max_jobs; i++){
          if (shell->jobs[i].jid == job_id){
            job = &shell->jobs[i];
            break;
          }
        }
      }
      else{
        pid_t pid = atoi(argv[1]);
        for (int i = 0; i < shell->max_jobs; i++){
          if (shell->jobs[i].pid == pid){
            job = &shell->jobs[i];
            break;
          }
        }
      }
      if (strcmp(argv[0], "bg") == 0){
        job->state = BACKGROUND;
        kill(-job->pid, SIGCONT);
      }
      else{
        job->state = FOREGROUND;
        kill(-job->pid, SIGCONT);
        waitfg(job->pid);
      }
    }
    return NULL;
  }
  else if (strcmp(argv[0], "kill") == 0){

    if (argv[1] != NULL && argv[2] != NULL){
      int sig_num = atoi(argv[1]);
      pid_t pid = atoi(argv[2]);
      if (sig_num == 2 || sig_num == 9 || sig_num == 18 || sig_num == 19){
        kill(-pid, sig_num);
      }
      else{
        printf("error: invalid signal number\n");
      }
    }
    return NULL;
  }
  return NULL;
}

int evaluate(msh_t *shell, char *line)
{

  if (strlen(line) > shell->max_line)
  {
    printf("error: reached the maximum line limit\n");
    return 1;
  }

  add_line_history(shell->history, line);

  int job_type;
  char *job;

  job = parse_tok(line, &job_type);

  while (job != NULL) {
    int argc;
    bool is_builtin;
    char **job_args = separate_args(job, &argc, &is_builtin);

    if (is_builtin){
      builtin_cmd(shell, job_args);
    }
    else{
      pid_t pid;
      int status;

      if (strstr(job, "exit") != NULL) {
        return 1;
      }

      sigset_t mask_all, mask_one, prev_one;  
      sigfillset(&mask_all);
      sigemptyset(&mask_one);
      sigaddset(&mask_one, SIGCHLD);

      if (job_type == 1){ // when the job is forground job

        sigprocmask(SIG_BLOCK, &mask_one, &prev_one); 

        if ((pid = fork()) == 0){
          setpgid(0, 0);
          sigprocmask(SIG_SETMASK, &prev_one, NULL);
          if (execve(job_args[0], job_args, NULL) < 0) { // child process being executed 
            exit(1);   
          } 
        }
        else{
          //job_t new_job;
          // new_job.state = FOREGROUND;
          // new_job.pid = pid;
          shell->foreground_pid = pid;
          sigprocmask(SIG_BLOCK, &mask_all, NULL);
          if (!add_job(shell->jobs, shell->max_jobs, pid, FOREGROUND, job)){
            printf("error: reached the maximum jobs limit\n");
          }
          sigprocmask(SIG_SETMASK, &prev_one, NULL);
          waitfg(pid); 
          //foreground_pid = 0;
        }

      }else if (job_type == 0){ // when the job is a background job

        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

        if ((pid = fork()) == 0){
          sigprocmask(SIG_SETMASK, &prev_one, NULL);
          if (execve(job_args[0], job_args, NULL) < 0) { // child process being executed
            exit(1);  
          } 
        }
        else{
          //job_t new_job;
          // new_job.state = BACKGROUND;
          // new_job.pid = pid;
          sigprocmask(SIG_BLOCK, &mask_all, NULL);
          if (!add_job(shell->jobs, shell->max_jobs, pid, BACKGROUND, job)){
            printf("error: reached the maximum jobs limit\n");
          }
          sigprocmask(SIG_SETMASK, &prev_one, NULL);
        }
      }
    }
    for (int i = 0; i < argc; i++){
      free(job_args[i]);
    }
    free(job_args);
    //free(job);
    job = parse_tok(NULL, &job_type); 

    if (job == NULL) {
      break;
    }
  }

  return 0;
}

void exit_shell(msh_t *shell)
{ 
  bool no_more_jobs(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].state == BACKGROUND) {
            return false; 
        }
    }
    return true;
  }

  while (1) {
    sleep(1);
    if (no_more_jobs(shell->jobs, shell->max_jobs)){
      break;
    }
  }

  free_jobs(shell->jobs, shell->max_jobs);

  free_history(shell->history);

  free(shell);
}