#ifndef _JOB_H_
#define _JOB_H_

#include <stdbool.h>
#include <sys/types.h>

typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell.
typedef struct job {
    char *cmd_line;     // The command line for this specific job.
    job_state_t state;  // The current state for this job
    pid_t pid;          // The process id for this job
    int jid;            // The job number for this job
}job_t;

/*
* alloc_jobs: allocate memory for the jobs list
*
* max_job: the maximum number of jobs should be allocated
*
* Returns: the allocated the jobs
*/
job_t *alloc_jobs(int max_jobs);
/*
* add_job: adds a new job to the array of jobs
* 
* jobs: The array of current jobs.
* 
* max_jobs: The maximum value of the number of jobs presented in the array.
*
* pid: The process id for the job to be added.
*
* state: The current state of the job yet to be added.
*
* cmd_line: The command line for this specific job to be added.
*
* Returns: A boolean value represents if the job has been successfully added
*/
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/*
* delete_job: removes a job from the current job array based on the pid provided
*
* jobs: The job array where a job need to be removed.
*
* max_jobs: the maximum number of jobs assigned to the shell.
*
* pid: The process id of the job that need to be removed from the job array.
*
* Returns: A boolean value that represents if the job is present in the array and successfully deleted.
*/
bool delete_job(job_t *jobs, int max_jobs, pid_t pid);

/*
* free_jobs: function to deallocate the job array
*
* jobs: The job array needs to be deallocated.
*
* max_jobs: The maximum number of jobs can be held in this job array.
*
* Returns: Nothing.
*/
void free_jobs(job_t *jobs, int max_jobs);

#endif