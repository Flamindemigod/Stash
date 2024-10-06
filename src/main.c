#include <stdio.h>

#include "manifest.h"
#define NOB_IMPLEMENTATION
#include "build/config.h"
#include "nob.h"

int main(int argc, char **argv) {
  Opts opts = {0};
  Manifest manifest = {0};
  int parseOptsRes = parseOpts(&opts, &argc, &argv);
  switch (parseOptsRes) {
  case FAILED_PARSING:
    return 1;
  case PARSED_NOW_EXIT:
    return 0;
  case PARSED_NOW_CONTINUE:
    break;
  }
  if (nob_file_exists(opts.manifest)) {
    parse_manifest(&opts, &manifest);
  } else {
    generate_example_manifest(&opts);
  }

  return 0;
}
