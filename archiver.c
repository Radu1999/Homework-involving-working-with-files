// Chivereanu Radu-Gabriel 315 CA
#include "mylib.h"

int main() {
  userinfo *owner;
  owner = malloc(sizeof(userinfo));
  char string[CHMAX], *word, *command;
  int count, maxword, i, it;
  char **parameters;
  it = getUserInformation(&owner);
  parameters = malloc(3 * sizeof(char *));
  while (true) {
    fgets(string, CHMAX, stdin);
    count = 0;
    maxword = 0;
    word = strtok(string, " \n");
    if (word == NULL) {
      printf("> Wrong command\n");
      continue;
    }
    command = word;
    if (!strcmp(word, "exit")) {
      exit(0);
    }
    maxword = numberOfCommandWords(word);
    while (word != NULL) {
      parameters[count] = malloc((strlen(word) + 1) * sizeof(char));
      strcpy(parameters[count++], word);
      if (count > maxword) {
        break;
      }
      word = strtok(NULL, " \n");
    }
    if (count != maxword) {
      printf("> Wrong command!\n");
    } else {
      if (!strcmp(command, "create")) {
        createTar(parameters, owner, it);
      }
      if (!strcmp(command, "list")) {
        list(parameters[1]);
      }
      if (!strcmp(command, "extract")) {
        extract(parameters[1], parameters[2]);
      }
    }
  }
  for (i = 0; i < 3; i++) {
    free(parameters[i]);
  }
  free(parameters);
  free(owner);
}
