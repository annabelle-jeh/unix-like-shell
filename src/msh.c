#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _GNU_SOURCE

int main(int argc, char **argv)
{
    int opt, max_jobs = 0, max_line = 0, max_history = 0;

    while ((opt = getopt(argc, argv, ":s:j:l:")) != -1)
        if (opt == 's' && atoi(optarg) > 0)
            max_history = atoi(optarg);
        else if (opt == 'j' && atoi(optarg) > 0)
            max_jobs = atoi(optarg);
        else if (opt == 'l' && atoi(optarg) > 0)
            max_line = atoi(optarg);
        else
        {
            printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
            return EXIT_FAILURE;
        }

    for (; optind < argc; ++optind)
    {
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return EXIT_FAILURE;
    }

    shell = alloc_shell(max_jobs, max_line, max_history);
    char *line = NULL;
    size_t length = 0;

    printf("msh> ");
    long nRead = getline(&line, &length, stdin);

    while (nRead != -1)
    {
        if (line[nRead - 1] == '\n')
            line[nRead - 1] = '\0';
        else
            line[nRead] = '\0';

        if (evaluate(shell, line) == 1)
            break;

        free(line);
        line = NULL;
        printf("msh> ");
        nRead = getline(&line, &length, stdin);
    }

    free(line);
    exit_shell(shell);
    return EXIT_SUCCESS;
}
