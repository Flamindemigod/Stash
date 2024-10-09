#include <stdbool.h>

typedef struct {
  char *program;
  char *manifest;
  char *version;
  char *homeDir;
  bool dryRun;
  bool forceReplace;
  bool unwind;
} Opts;

#define FAILED_PARSING 0
#define PARSED_NOW_EXIT 1
#define PARSED_NOW_CONTINUE 2

int parseOpts(Opts *opts, int *argc, char ***argv);
