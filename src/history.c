#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";

history_t *alloc_history(int max_history)
{
    history_t *history = malloc(sizeof(history_t));

    history->lines = malloc(max_history * sizeof(char *));
    history->max_history = max_history;
    history->next = 0;

    FILE *file = fopen(HISTORY_FILE_PATH, "r");

    if (file != NULL)
    {
        char *line = NULL;
        size_t length = 0;
        long nRead = getline(&line, &length, file);

        while (nRead != -1 && history->next < max_history)
        {
            line[nRead - 1] = '\0';
            history->lines[history->next++] = strdup(line);

            free(line);
            line = NULL;
            nRead = getline(&line, &length, file);
        }

        free(line);
        fclose(file);
    }

    return history;
}

void add_line_history(history_t *history, const char *cmd_line)
{
    int N = 0;

    if (cmd_line != NULL && strcmp(cmd_line, "exit") != 0 && sscanf(cmd_line, "!%d", &N) != 1)
    {
        char *line_copy = strdup(cmd_line);

        if (strstr(line_copy, "exit") != NULL)
            *strstr(line_copy, "exit") = '\0';

        if (line_copy[0] != '\0')
        {
            if (history->next == history->max_history)
            {
                free(history->lines[0]);

                for (int i = 0; i < history->max_history - 1; ++i)
                    history->lines[i] = history->lines[i + 1];

                history->next--;
            }

            history->lines[history->next++] = strdup(line_copy);
        }

        free(line_copy);
    }
}

void print_history(history_t *history)
{
    for (int i = 0; i < history->next; ++i)
        printf("%5d\t%s\n", i + 1, history->lines[i]);
}

char *find_line_history(history_t *history, int index)
{
    if (index > 0 && index <= history->next)
        return history->lines[index - 1];

    return NULL;
}

void free_history(history_t *history)
{
    FILE *file = fopen(HISTORY_FILE_PATH, "w");

    if (file != NULL)
    {
        for (int i = 0; i < history->next; ++i)
            fprintf(file, "%s\n", history->lines[i]);

        fclose(file);
    }

    for (int i = 0; i < history->next; ++i)
        free(history->lines[i]);

    free(history->lines);
    free(history);
}
