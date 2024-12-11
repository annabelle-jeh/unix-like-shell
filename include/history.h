#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

// Represents the state of the history of the shell
typedef struct history
{
    char **lines;
    int max_history;
    int next;
} history_t;

/*
 * alloc_history: Allocates and initializes the state of the shell history with the history file
 *
 * max_history: The maximum number of saved history commands for the shell
 *
 * Returns: a history_t pointer that is allocated and initialized.
 */

history_t *alloc_history(int max_history);

/*
 * add_line_history: Adds a new command line to the shell history
 *
 * history: The current state of the history of the shell
 *
 * cmd_line: The command line to be added to the shell history
 */

void add_line_history(history_t *history, const char *cmd_line);

/*
 * print_history: Prints all the command lines in the shell history
 *
 * history: The current state of the history of the shell
 */

void print_history(history_t *history);

/*
 * find_line_history: Finds the command line at this index in the shell history
 *
 * history: The current state of the history of the shell
 *
 * index: The index of the command line to be found in the shell history
 *
 * Returns: the command line at the specified index in the shell history.
 */

char *find_line_history(history_t *history, int index);

/*
 * free_history: Writes to the history file and deallocates the state of the shell history
 *
 * history: The current state of the history of the shell
 */

void free_history(history_t *history);

#endif
