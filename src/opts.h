#include <stdbool.h>

typedef struct {
  char *program;
  char *manifest;
  char *version;
} Opts;

bool parseOpts(Opts *opts, int *argc, char ***argv);
