#include "signal_handlers.h"
#include "job.h"
#include "shell.h"
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

extern msh_t *shell;
/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other
*     currently running children to terminate.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/

int getjid(pid_t pid) {
  if (pid < 1)
	  return 0;
  for (int i = 0; i < shell->max_jobs; i++)
	  if (shell->jobs[i].pid == pid) {
      return shell->jobs[i].jid;
    }
  return 0;
}

void sigchld_handler(int sig)
{
  int olderrno = errno;
  sigset_t mask_all, prev_all;
  int status;
  pid_t pid;

  sigfillset(&mask_all);

  if ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    for (int i = 0; i < shell->max_jobs; i++){
      if (shell->jobs[i].state == FOREGROUND){
        foreground_pid = pid;
      }
    }
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    if (WIFEXITED(status)) {
      delete_job(shell->jobs, shell->max_jobs, pid);
    }
    if (WIFSIGNALED(status)){
      delete_job(shell->jobs, shell->max_jobs, pid);
    }
    else if (WIFSTOPPED(status)){
      for (int i = 0; i < shell->max_jobs; i++){
        if (shell->jobs[i].pid == pid){
          shell->jobs[i].state = SUSPENDED;
        }
      }
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
  }
  errno = olderrno;
  return;
}

/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigint_handler(int sig)
{ 
  if (shell->foreground_pid != -1){
    kill(-shell->foreground_pid, SIGINT);
    delete_job(shell->jobs, shell->max_jobs, foreground_pid);
  }
  return;
}

/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigtstp_handler(int sig)
{
  if (shell->foreground_pid != -1){
    kill(-shell->foreground_pid, SIGTSTP);
  }
  return;
}

/*
* setup_handler - wrapper for the sigaction function
*
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("Signal error");
        exit(1);
    }
    return (old_action.sa_handler);
}

void initialize_signal_handlers() {

    // sigint handler: Catches SIGINT (ctrl-c) signals.
    setup_handler(SIGINT,  sigint_handler);   /* ctrl-c */
    // sigtstp handler: Catches SIGTSTP (ctrl-z) signals.
    setup_handler(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    // sigchld handler: Catches SIGCHILD signals.
    setup_handler(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
}