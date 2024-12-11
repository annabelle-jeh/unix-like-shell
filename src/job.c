#include "job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

job_t *alloc_jobs(int max_jobs, int max_line)
{
    job_t *jobs = malloc(max_jobs * sizeof(job_t));

    for (int i = 0; i < max_jobs; ++i)
    {
        jobs[i].cmd_line = malloc(max_line * sizeof(char));
        jobs[i].state = UNDEFINED;
        jobs[i].pid = -1;
        jobs[i].jid = 0;
    }

    return jobs;
}

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line)
{
    for (int i = 0; i < max_jobs; ++i)
        if (jobs[i].pid == -1)
        {
            strcpy(jobs[i].cmd_line, cmd_line);
            jobs[i].state = state;
            jobs[i].pid = pid;
            jobs[i].jid = i + 1;
            return true;
        }

    return false;
}

bool delete_job(job_t *jobs, int max_jobs, pid_t pid)
{
    for (int i = 0; i < max_jobs; ++i)
        if (jobs[i].pid == pid)
        {
            jobs[i].cmd_line[0] = '\0';
            jobs[i].state = UNDEFINED;
            jobs[i].pid = -1;
            jobs[i].jid = 0;
            return true;
        }

    return false;
}

void free_jobs(job_t *jobs, int max_jobs)
{
    for (int i = 0; i < max_jobs; ++i)
        free(jobs[i].cmd_line);

    free(jobs);
}
