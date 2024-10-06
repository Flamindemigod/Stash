#include <stdio.h>

#include "manifest.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "build/config.h"


int main(int argc, char **argv) {
  Opts opts = {0};
  if (!parseOpts(&opts, &argc, &argv)){
    nob_log(NOB_ERROR, "Failed to parse command line options\n");
    return 1;
  }
  if (nob_file_exists(opts.manifest))
  {
  parse_manifest(&opts);
  } else{
    generate_example_manifest(&opts);
  }
  
  return 0;
}
