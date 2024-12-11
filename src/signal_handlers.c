#include "signal_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

void sigchld_handler(int sig)
{
    int olderrno = errno;
    pid_t child_pid;
    int child_status;
    sigset_t mask_all, prev_all;

    while ((child_pid = waitpid(-1, &child_status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);

        if (child_pid == -1)
        {
            perror("wait error");
            exit(1);
        }

        else if (WIFEXITED(child_status))
        {
            for (int i = 0; i < shell->max_jobs; ++i)
                if (shell->jobs[i].pid == child_pid && shell->jobs[i].state != SUSPENDED)
                    if (delete_job(shell->jobs, shell->max_jobs, child_pid))
                    {
                        fg_pid = (child_pid == fg_pid) ? -1 : fg_pid;
                        job_count--;
                        break;
                    }
        }

        else if (WIFSIGNALED(child_status) && (WTERMSIG(child_status) == SIGINT || WTERMSIG(child_status) == SIGKILL))
        {
            for (int i = 0; i < shell->max_jobs; ++i)
                if (shell->jobs[i].pid == child_pid)
                    if (delete_job(shell->jobs, shell->max_jobs, child_pid))
                    {
                        fg_pid = (child_pid == fg_pid) ? -1 : fg_pid;
                        job_count = (shell->jobs[i].state != SUSPENDED) ? job_count - 1 : job_count;
                        break;
                    }
        }

        else if (WIFSTOPPED(child_status) && (WSTOPSIG(child_status) == SIGTSTP || WSTOPSIG(child_status) == SIGSTOP))
        {
            for (int i = 0; i < shell->max_jobs; ++i)
                if (shell->jobs[i].pid == child_pid && shell->jobs[i].state != SUSPENDED)
                {
                    shell->jobs[i].state = SUSPENDED;
                    fg_pid = (child_pid == fg_pid) ? -1 : fg_pid;
                    job_count--;
                    break;
                }
        }

        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    errno = olderrno;
}

void sigint_handler(int sig)
{
    kill(-fg_pid, SIGINT);
}

void sigtstp_handler(int sig)
{
    kill(-fg_pid, SIGTSTP);
}

handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
    {
        perror("signal error");
        exit(1);
    }

    return old_action.sa_handler;
}

void initialize_signal_handlers()
{
    // SIGINT handler: Catches SIGINT (ctrl-c) signals
    setup_handler(SIGINT, sigint_handler); /* ctrl-c */

    // SIGTSTP handler: Catches SIGTSTP (ctrl-z) signals
    setup_handler(SIGTSTP, sigtstp_handler); /* ctrl-z */

    // SIGCHILD handler: Catches SIGCHILD signals
    setup_handler(SIGCHLD, sigchld_handler); /* Terminated or stopped child */
}
