#ifndef _SHELL_H_
#define _SHELL_H_

#include "job.h"
#include "history.h"
#include "signal_handlers.h"
#include <stdbool.h>

// Represents the state of the shell
typedef struct msh
{
    int max_jobs;
    int max_line;
    int max_history;
    job_t *jobs;
    history_t *history;
} msh_t;

extern msh_t *shell;
extern volatile int job_count;
extern volatile sig_atomic_t fg_pid;

/*
 * alloc_shell: Allocates and initializes the state of the shell
 *
 * max_jobs: The maximum number of jobs that can be in existence at any point in time
 *
 * max_line: The maximum number of characters that can be entered for any specific command line
 *
 * max_history: The maximum number of saved history commands for the shell
 *
 * Returns: a msh_t pointer that is allocated and initialized.
 */

msh_t *alloc_shell(int max_jobs, int max_line, int max_history);

/*
 * parse_tok: Continuously retrieves separate commands from the provided command line until all commands are parsed
 *
 * line: The command line to parse, which may include multiple commands. If line is NULL then parse_tok continues parsing the previous command line
 *
 * job_type: Specifies whether the parsed command is a background (sets the value of 0 at the address of job_type) or foreground job (sets the value of 1 at the address of job_type). If no job is returned then assign the value at the address to -1
 *
 * Returns: NULL no other commands can be parsed; otherwise, it returns a parsed command from the command line.
 */

char *parse_tok(char *line, int *job_type);

/*
 * separate_args: Separates the arguments of command and places them in an allocated array returned by this function
 *
 * line: The command line to separate. This function assumes only a single command that takes in zero or more arguments
 *
 * argc: Stores the number of arguments produced at the memory location of the argc pointer
 *
 * is_builtin: True if the command is a built-in command; otherwise false
 *
 * Returns: NULL is line contains no arguments; otherwise, a newly allocated array of strings that represents the arguments of the command (similar to argv). Make sure the array includes a NULL value in its last location.
 */

char **separate_args(char *line, int *argc, bool *is_builtin);

/*
 * evaluate: Executes the provided command line string
 *
 * shell: The current shell state value
 *
 * line: The command line string to evaluate
 *
 * Returns: non-zero if the command executed wants the shell program to close. Otherwise, a 0 is returned.
 */

int evaluate(msh_t *shell, char *line);

/*
 * exit_shell: Closes down the shell by deallocating the shell state.
 *
 * shell: The current shell state value
 */

void exit_shell(msh_t *shell);

/*
 * builtin_cmd: Handles the processing of any built-in commands
 *
 * argc: The number of arugments being currently processed by the shell
 *
 * argv: The arugment array being currently processed by the shell
 *
 * Returns: !N returns the command line history. Otherwise, returns NULL to indicate nothing additional happens.
 */

char *builtin_cmd(int argc, char **argv);

#endif
