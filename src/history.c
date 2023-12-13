#include "history.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";

history_t *alloc_history(int max_history){

  history_t* new_history = (history_t*)malloc(sizeof(history_t));

  new_history->lines = (char**)malloc(sizeof(char*)* max_history);

  // if (new_history->lines == NULL) {
  //   perror("Failed to allocate memory for history lines");
  //   free(new_history);
  //   return NULL;
  // }

  new_history->max_history = max_history;
  new_history->next = 0;

  FILE* history_file = fopen(HISTORY_FILE_PATH, "r");

  if (history_file != NULL){
    int line_count = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), history_file) != NULL){
      buffer[strcspn(buffer, "\n")] = '\0';
      new_history->lines[line_count] = strdup(buffer);
      line_count++;

      if (line_count == max_history){
        break;
      }
    }
    
    fclose(history_file);

    new_history->next = line_count;
  }

  return new_history;
}


void add_line_history(history_t *history, const char *cmd_line){

  if (cmd_line == NULL || cmd_line[0] == '\0' || strcmp(cmd_line, "exit") == 0 || (cmd_line[0] == '!' && isdigit(cmd_line[1]))){
    return;
  }

  if (history->next == history->max_history) {
    free(history->lines[0]);

    for (int i = 0; i < history->max_history - 1; i++) {
      history->lines[i] = history->lines[i + 1];
    }

    history->next--;
  }

  history->lines[history->next] = strdup(cmd_line);
  history->next++;
}

void print_history(history_t *history) {
  for(int i = 1; i <= history->next; i++) {
    printf("%5d\t%s\n",i,history->lines[i-1]);
  }
}

char *find_line_history(history_t *history, int index){
  if (index >= 1 && index <= history->next) {
    return history->lines[index - 1];
  } 
  else {
    return NULL;
  }
}

void free_history(history_t *history) {

  FILE *history_file = fopen(HISTORY_FILE_PATH, "w");

  if (history_file != NULL) {  // Check if fopen was successful
    for (int i = 0; i < history->next; i++) {
      fprintf(history_file, "%s\n", history->lines[i]);
    }
    fclose(history_file);
  }

  for (int i = 0; i < history->next; i++) {
    free(history->lines[i]);
  }

  free(history->lines);
  free(history);
}