#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

//Represents the state of the history of the shell
typedef struct history {
    char **lines;
    int max_history;
    int next;
}history_t;

/*
* alloc_history: allocate memory for history_t
* 
* max_history: the maximum number of history_t that will be stored in the file 
* 
* Returns: the allocated history_t state
*/
history_t *alloc_history(int max_history);

/*
* add_line_history: adds the parameter cmd_line to the history of lines, if the history lines are full,
* remove the command line at index 0 and move all the command lines foward by 1
* 
* history: the original history
*
* cmd_line: the to be added command line
*
* Returns: nothing
*/
void add_line_history(history_t *history, const char *cmd_line);

/*
* print_history: print the history
*
* history: the history stored
*
* Return: nothing
*/
void print_history(history_t *history);

/*
* find_line_history: used to find a specific command line(job) at the given index
* 
* history: the history stored
*
* index: the index of the command line to be find
*
* Return: the command line at the specified index
*/
char *find_line_history(history_t *history, int index);

/*
* free_history: frees the memory allocated to the history
*
* history: the history allocated
*
* Retunr: nothing
*/
void free_history(history_t *history);

#endif