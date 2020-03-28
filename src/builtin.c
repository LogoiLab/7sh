#include "builtin.h"

static char *commands[] = {"exit", "egg", "help"};
static int num_commands = 3;

bool is_builtin(char *name) {
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(name, commands[i]) == 0) {
      return true;
    }
  }
  return false;
}

void exec_builtin(char *args[]) {
  int selected_command = 0;
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(args[0], commands[i]) == 0) {
      selected_command = i;
    }
  }
  switch(selected_command) {
  case 0: {
    exit(EXIT_SUCCESS);
    break;
  }
  case 1: {
    fprintf(stdout, "_________  _________ ___ ______________.____    .____     \n\\______  \\/   _____//   |   \\_   _____/|    |   |    |    \n    /    /\\_____  \\/    ~    \\    __)_ |    |   |    |    \n   /    / /        \\    Y    /        \\|    |___|    |___ \n  /____/ /_______  /\\___|_  /_______  /|_______ \\_______ \\\n                 \\/       \\/        \\/         \\/       \\/\n");
  }
  case 2: {
    fprintf(stdout, "7-Shell: v0.1.0 ©DosLab Electronics, LLC\nLicensed Under GNU GPL3\n\nUsage:\t7sh <prompt_string>\n");
  }
  }
}
