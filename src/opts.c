#include "build/config.h"
#include "manifest.h"
#include "nob.h"

#define MATCH_ARG(arg) (strcmp(subcommand, arg) == 0)

void init_opts(Opts *opts) {
  opts->manifest = "./.MANIFEST";
  opts->dryRun = false;
  opts->forceReplace = false;
  opts->homeDir = false;
  opts->unwind = false;
  opts->noPreservePerm = false;
  if (getuid() == 0) {
    opts->isRoot = true;
  } else {
    opts->isRoot = false;
  }
#ifdef VERSION
  opts->version = VERSION;
#else
  opts->version = "dev-unknown-commit";
#endif
}

void printUsage(Opts *opts) {
  Nob_String_Builder sb = {0};
  nob_sb_append_cstr(&sb, "Using Stash\n");
  nob_sb_append_cstr(
      &sb, nob_temp_sprintf("%s [-h | --help] [-v | --version] {...Options}\n",
                            opts->program));
  nob_sb_append_cstr(&sb, "Options\n");
  nob_sb_append_cstr(&sb, "\t--manifest {MANIFEST}\t\tDefault: ./.MANIFEST"
                          "\n\t\t\t\t\tConflicts with --generate-manifest\n\n");
  nob_sb_append_cstr(&sb,
                     "\t--generate-manifest {MANIFEST}\tDefault: ./.MANIFEST"
                     "\n\t\t\t\t\tConflicts with --manifest\n\n");
  nob_sb_append_cstr(&sb, "\t-f   --force\t\t\tForce replace links\n\n");
  nob_sb_append_cstr(&sb, "\t-g   --home\t\t\tPath to your home directory\n\n");
  nob_sb_append_cstr(&sb, "\t--dry-run\t\t\t"
                          "No file changes take place.\n\t\t\t\t\t"
                          "Just outputs what links where\n\n");
  nob_sb_append_cstr(&sb, "\t--no-preserve-perm\t\t"
                          "Used when running as root and to link files to non "
                          "root locations.\n\t\t\t\t\t"
                          "This however sets files to be owned by root\n");
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

int parseOpts(Opts *opts, int *argc, char ***argv) {
  int result = PARSED_NOW_CONTINUE;
  init_opts(opts);
  opts->program = nob_shift_args(argc, argv);
  while (*argc > 0) {
    char *subcommand = nob_shift_args(argc, argv);
    if (MATCH_ARG("--help") || MATCH_ARG("-h")) {
      printUsage(opts);
      nob_return_defer(PARSED_NOW_EXIT);
    }
    if (MATCH_ARG("--version") || MATCH_ARG("-v")) {
      printVersion(opts);
      nob_return_defer(PARSED_NOW_EXIT);
    }
    if MATCH_ARG ("--manifest") {
      if (*argc <= 0) {
        nob_log(NOB_ERROR, "--manifest requires a path");
        printUsage(opts);
        nob_return_defer(FAILED_PARSING);
      } else {
        char *manifest_path = nob_shift_args(argc, argv);
        if (nob_file_exists(manifest_path)) {
          opts->manifest = manifest_path;
        } else {
          nob_log(NOB_ERROR, "%s does not exist.", manifest_path);
          nob_log(NOB_INFO, "if you want to generate a config at %s",
                  manifest_path);
          nob_log(NOB_INFO, "run '%s --generate-manifest %s'", opts->program,
                  manifest_path);
          nob_return_defer(FAILED_PARSING);
        }
      }
    } else if MATCH_ARG ("--generate-manifest") {
      char *manifest_path;
      if (*argc <= 0) {
        manifest_path = opts->manifest;
      } else {
        manifest_path = nob_shift_args(argc, argv);
      }
      if (nob_file_exists(manifest_path)) {
        nob_log(NOB_ERROR, "%s exists.", manifest_path);
        nob_log(NOB_INFO, "if you want to use a existing config",
                manifest_path);
        nob_log(NOB_INFO, "run '%s --manifest %s'", opts->program,
                manifest_path);
        nob_return_defer(FAILED_PARSING);
      } else {
        opts->manifest = manifest_path;
        generate_example_manifest(opts);
        nob_return_defer(PARSED_NOW_EXIT);
      }

    } else if MATCH_ARG ("--dry-run") {
      opts->dryRun = true;
    } else if MATCH_ARG ("--no-preserve-perm") {
      opts->noPreservePerm = true;
      if (!opts->isRoot) {
        nob_log(NOB_WARNING, "--no-preserve-perm is a root only permission. It "
                             "will not do anything when run as non-root");
      }
    } else if (MATCH_ARG("--home") || MATCH_ARG("-g")) {
      if (*argc <= 0) {
        nob_log(NOB_ERROR, "--home or -g requires a path");
        printUsage(opts);
        nob_return_defer(FAILED_PARSING);
      }

      char *home_path = nob_shift_args(argc, argv);
      if (!nob_file_exists(home_path)) {
        nob_log(NOB_ERROR, "The home path %s doesnt exist", home_path);
        nob_return_defer(FAILED_PARSING);
      }
      opts->homeDir = home_path;
    } else if (MATCH_ARG("--force") || MATCH_ARG("-f")) {
      opts->forceReplace = true;
    } else {
      nob_log(NOB_ERROR, "Invalid Subcommand %s", subcommand);
      printUsage(opts);
      nob_return_defer(FAILED_PARSING);
    }
  }
defer:
  return result;
}
