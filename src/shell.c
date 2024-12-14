#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_JOBS 16
#define MAX_HISTORY 10

msh_t *shell = NULL;
volatile int job_count = 0;
volatile sig_atomic_t fg_pid = -1;

msh_t *alloc_shell(int max_jobs, int max_line, int max_history)
{
    msh_t *shell = malloc(sizeof(msh_t));

    shell->max_jobs = (max_jobs != 0) ? max_jobs : MAX_JOBS;
    shell->max_line = (max_line != 0) ? max_line : MAX_LINE;
    shell->max_history = (max_history != 0) ? max_history : MAX_HISTORY;
    shell->jobs = alloc_jobs(shell->max_jobs, shell->max_line);
    shell->history = alloc_history(shell->max_history);
    initialize_signal_handlers();

    return shell;
}

char *parse_tok(char *line, int *job_type)
{
    static char *pointer = NULL;
    char *delimiter, *token;

    if (line != NULL)
        pointer = line;

    if (pointer != NULL)
        for (int i = 0; pointer[i] != '\0'; ++i)
            if (pointer[i] != ' ')
            {
                delimiter = strpbrk(pointer, ";&");
                *job_type = (delimiter != NULL && *delimiter == '&') ? 0 : 1;

                token = strtok_r(pointer, ";&", &pointer);
                return token;
            }

    *job_type = -1;
    return NULL;
}

char **separate_args(char *line, int *argc, bool *is_builtin)
{
    *argc = 0;
    *is_builtin = false;

    char **argv = malloc(sizeof(char *));
    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " ");

    while (token != NULL)
    {
        argv = realloc(argv, (++(*argc) + 1) * sizeof(char *));
        argv[*argc - 1] = strdup(token);
        token = strtok(NULL, " ");
    }

    free(line_copy);

    if (*argc == 0)
    {
        free(argv);
        return NULL;
    }

    argv[*argc] = NULL;
    char *builtin_line = builtin_cmd(*argc, argv);

    if (builtin_line == NULL)
        *is_builtin = true;

    else if (strcmp(builtin_line, "not built-in") != 0)
    {
        *is_builtin = true;

        char *line_copy = strdup(builtin_line);
        evaluate(shell, line_copy);
        free(line_copy);
    }

    return argv;
}

int evaluate(msh_t *shell, char *line)
{
    bool is_builtin;
    int type, argc, child_status;
    pid_t child_pid, fg_term_pid, bg_term_pid;
    sigset_t mask_all, mask_one, prev_one;

    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    if (strlen(line) > shell->max_line)
    {
        printf("error: reached the maximum line limit\n");
        return 0;
    }

    add_line_history(shell->history, line);
    char *job = parse_tok(line, &type);

    while (job != NULL)
    {
        job_state_t child_state = (type == 0) ? BACKGROUND : FOREGROUND;
        char **argv = separate_args(job, &argc, &is_builtin);

        if (argc == 1 && strcmp(argv[0], "exit") == 0)
        {
            free(argv[0]);
            free(argv);
            return 1;
        }

        if (!is_builtin)
        {
            sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

            if ((child_pid = fork()) == 0)
            {
                setpgid(0, 0);
                sigprocmask(SIG_SETMASK, &prev_one, NULL);
                execve(argv[0], argv, NULL);
                exit(0);
            }

            sigprocmask(SIG_BLOCK, &mask_all, NULL);

            if (child_state == FOREGROUND)
                fg_pid = child_pid;

            if (add_job(shell->jobs, shell->max_jobs, child_pid, child_state, job))
                job_count++;

            sigprocmask(SIG_SETMASK, &prev_one, NULL);

            while (fg_pid != -1)
                sleep(1);
        }

        for (int i = 0; i < argc; ++i)
            free(argv[i]);

        free(argv);
        job = parse_tok(NULL, &type);
    }

    return 0;
}

void exit_shell(msh_t *shell)
{
    while (job_count > 0)
        sleep(1);

    free_jobs(shell->jobs, shell->max_jobs);
    free_history(shell->history);
    free(shell);
}

char *builtin_cmd(int argc, char **argv)
{
    int N = 0, pid = 0, jid = 0;
    char signal[MAX_LINE] = "\0";

    /* built-in 1: jobs command */
    if (argc == 1 && strcmp(argv[0], "jobs") == 0)
    {
        for (int i = 0; i < shell->max_jobs; ++i)
            if (shell->jobs[i].pid != -1)
            {
                if (shell->jobs[i].state != SUSPENDED)
                    printf("[%d] %d RUNNING %s\n", shell->jobs[i].jid, shell->jobs[i].pid, shell->jobs[i].cmd_line);
                else
                    printf("[%d] %d STOPPED %s\n", shell->jobs[i].jid, shell->jobs[i].pid, shell->jobs[i].cmd_line);
            }
        return NULL;
    }

    /* built-in 2: history command */
    else if (argc == 1 && strcmp(argv[0], "history") == 0)
    {
        print_history(shell->history);
        return NULL;
    }

    /* built-in 3: !N command */
    else if (argc == 1 && sscanf(argv[0], "!%d", &N) == 1)
    {
        return find_line_history(shell->history, N);
    }

    /* built-in 4: bg <job> command */
    else if (argc == 2 && strcmp(argv[0], "bg") == 0)
    {
        if (sscanf(argv[1], "%d", &pid) == 1)
            pid = pid;

        else if (sscanf(argv[1], "%%%d", &jid) == 1)
            pid = shell->jobs[jid - 1].pid;

        for (int i = 0; i < shell->max_jobs; ++i)
            if (shell->jobs[i].pid == pid && shell->jobs[i].state == SUSPENDED)
            {
                shell->jobs[i].state = BACKGROUND;
                job_count++;
                break;
            }

        kill(-pid, SIGCONT);

        return NULL;
    }

    /* built-in 5: fg <job> command */
    else if (argc == 2 && strcmp(argv[0], "fg") == 0)
    {
        if (sscanf(argv[1], "%d", &pid) == 1)
            fg_pid = pid;

        else if (sscanf(argv[1], "%%%d", &jid) == 1)
            fg_pid = shell->jobs[jid - 1].pid;

        for (int i = 0; i < shell->max_jobs; ++i)
            if (shell->jobs[i].pid == fg_pid && shell->jobs[i].state != FOREGROUND)
            {
                shell->jobs[i].state = FOREGROUND;
                job_count = (shell->jobs[i].state == SUSPENDED) ? job_count + 1 : job_count;
                break;
            }

        kill(-fg_pid, SIGCONT);

        while (fg_pid != -1)
            sleep(1);

        return NULL;
    }

    /* built-in 6: kill SIG_NUM PID command */
    else if (argc == 3 && strcmp(argv[0], "kill") == 0 && sscanf(argv[1], "%s", signal) == 1)
    {
        if (sscanf(argv[2], "%d", &pid) == 1)
            pid = pid;

        else if (sscanf(argv[2], "%%%d", &jid) == 1)
            pid = shell->jobs[jid - 1].pid;

        if (atoi(signal) == 2 || strcmp(signal, "SIGINT") == 0)
            kill(-pid, SIGINT);

        else if (atoi(signal) == 9 || strcmp(signal, "SIGKILL") == 0)
            kill(-pid, SIGKILL);

        else if (atoi(signal) == 18 || strcmp(signal, "SIGCONT") == 0)
        {
            for (int i = 0; i < shell->max_jobs; ++i)
                if (shell->jobs[i].pid == pid && shell->jobs[i].state == SUSPENDED)
                {
                    shell->jobs[i].state = BACKGROUND;
                    job_count++;
                    break;
                }

            kill(-pid, SIGCONT);
        }

        else if (atoi(signal) == 19 || strcmp(signal, "SIGSTOP") == 0)
            kill(-pid, SIGSTOP);

        else
            printf("error: invalid signal number\n");

        return NULL;
    }

    return "not built-in";
}
