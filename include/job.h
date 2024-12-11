#ifndef _JOB_H_
#define _JOB_H_

#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum job_state {FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell
typedef struct job
{
    char *cmd_line;
    job_state_t state;
    pid_t pid;
    int jid;
} job_t;

/*
 * alloc_jobs: Allocates and initializes the array of jobs that are running in the shell
 *
 * max_jobs: The maximum number of jobs that can be in existence at any point in time
 *
 * max_line: The maximum number of characters that can be entered for any specific command line
 *
 * Returns: a job_t pointer that is allocated and initialized.
 */

job_t *alloc_jobs(int max_jobs, int max_line);

/*
 * add_job: Adds a new job to the array of jobs that are running in the shell
 *
 * jobs: The array of jobs that are running in the shell
 *
 * max_jobs: The maximum number of jobs that can be in existence at any point in time
 *
 * pid: The pid of the job to be added to the array
 *
 * state: The state of the job to be added to the array
 *
 * cmd_line: The command line of the job to be added to the array
 *
 * Returns: True if the job has been added. Otherwise, false if there are no more jobs left to allocate.
 */

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/*
 * delete_job: Removes a job from the array of jobs based on the pid_t provided
 *
 * jobs: The array of jobs that are running in the shell
 *
 * max_jobs: The maximum number of jobs that can be in existence at any point in time
 *
 * pid: The pid of the job to be removed from the array
 *
 * Returns: True if the job with the provided pid_t has been removed. Otherwise, false if the job does not exist.
 */

bool delete_job(job_t *jobs, int max_jobs, pid_t pid);

/*
 * free_jobs: Deallocates the array of jobs in the shell
 *
 * jobs: The array of jobs that are running in the shell
 *
 * max_jobs: The maximum number of jobs that can be in existence at any point in time
 */

void free_jobs(job_t *jobs, int max_jobs);

#endif
