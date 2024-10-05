//#include <assert.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

int build_stash(Nob_Cmd *cmd) {
  int retVal = 1;
  if (nob_needs_rebuild1("./build/stash", "./main.c")){
  nob_cmd_append(cmd, "cc");
  nob_cmd_append(cmd, "-Wall", "-Wextra", "-ggdb");
  nob_cmd_append(cmd, "-o", "./build/stash");
  nob_cmd_append(cmd, "main.c");
  retVal = nob_cmd_run_sync(*cmd);
  }
  cmd->count = 0;
  return retVal;
}

int run_stash(Nob_Cmd *cmd) {
  nob_log(NOB_INFO, "Running Stash");
  nob_cmd_append(cmd, "./build/stash");
  int retVal = nob_cmd_run_sync(*cmd);
  cmd->count = 0;
  return retVal;
}

void generate_build_folder() { nob_mkdir_if_not_exists("./build"); }

int main(int argc, char *argv[]) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  int result = 0;
  const char *program = nob_shift_args(&argc, &argv);
  generate_build_folder();
  Nob_Cmd cmd = {0};
  if (!build_stash(&cmd)) {
    nob_log(NOB_ERROR, "Failed to build stash\nExiting\n");
    nob_return_defer(EXIT_FAILURE);
  }

  if (argc <= 0){
    nob_log(NOB_INFO, "Exiting");
    nob_return_defer(EXIT_SUCCESS);
  }
  const char *subcommand = nob_shift_args(&argc, &argv);
  nob_log(NOB_INFO, "%s", subcommand);
  if (strcmp(subcommand, "run") == 0){
    run_stash(&cmd);
  }
  defer:
    nob_cmd_free(cmd);
    return result;
}
