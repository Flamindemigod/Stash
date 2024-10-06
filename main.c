#include <stdio.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "build/config.h"

typedef struct {
  char *program;
  char *manifest;
  char *version;
} Opts;

#define MATCH_ARG(arg) (strcmp(subcommand, arg) == 0)

void printUsage(Opts *opts) {
  Nob_String_Builder sb = {0};
  nob_sb_append_cstr(&sb, "Using Stash\n");
  nob_sb_append_cstr(
      &sb, nob_temp_sprintf("%s [-h | --help] [-v | --version] {...Options}\n",
                            opts->program));
  nob_sb_append_cstr(&sb, "\tOptions\n");
  nob_sb_append_cstr(&sb,
                     "\t\t--manifest {MANIFEST}\t\tDefault: ./.MANIFEST\n");
  nob_sb_append_null(&sb);
  fprintf(stderr, "%s", sb.items);
  nob_sb_free(sb);
}

void printVersion(Opts *opts) {
  Nob_String_Builder sb = {0};
  nob_sb_append_cstr(&sb, nob_temp_sprintf("Version: %s\n", opts->version));
  nob_sb_append_null(&sb);
  fprintf(stderr, "%s", sb.items);
  nob_sb_free(sb);
}

void init_opts(Opts *opts) {
  opts->manifest = "./.MANIFEST";
#ifdef VERSION
  opts->version = VERSION;
#else
  opts->version = "dev-unknown-commit";
#endif
}

int main(int argc, char **argv) {
  Opts opts = {0};
  init_opts(&opts);
  opts.program = nob_shift_args(&argc, &argv);
  while (argc > 0) {
    char *subcommand = nob_shift_args(&argc, &argv);
    if (MATCH_ARG("--help") || MATCH_ARG("-h")) {
      printUsage(&opts);
      return 0;
    } else if (MATCH_ARG("--version") || MATCH_ARG("-v")) {
      printVersion(&opts);
      return 0;
    } else if MATCH_ARG ("--manifest") {
      if (argc <= 0) {
        nob_log(NOB_ERROR, "--manifest requires a path");
        printUsage(&opts);
        return 1;
      } else {
        char *manifest_path = nob_shift_args(&argc, &argv);
        if (nob_file_exists(manifest_path)) {
          opts.manifest = manifest_path;
        } else {
          nob_log(NOB_ERROR, "%s does not exist. Exiting", manifest_path);
          return 1;
        }
      }
    } else {
      nob_log(NOB_ERROR, "Invalid Subcommand");
      printUsage(&opts);
      return 1;
    }
  }
  return 0;
}
