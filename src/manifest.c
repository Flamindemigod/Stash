#include "nob.h"
#include "opts.h"
#include <stdio.h>
#include <unistd.h>

#define WRITE(str) fputs(str, fd);

enum LINKMODE { SYMLINK };

typedef struct {
  char *src;
  char *dest;
  enum LINKMODE mode;
} ManifestField;

void parse_manifest(Opts *opts) {
  Nob_String_Builder sb = {0};
  nob_read_entire_file(opts->manifest, &sb);
  printf("%s\n", sb.items);
}

bool generate_example_manifest(Opts *opts) {
  bool result = true;
  nob_log(NOB_INFO, "Generating Example Manifest file at %s", opts->manifest);
  FILE *fd = fopen(opts->manifest, "wb");
  if (fd == NULL) {
    nob_return_defer(false);
  }
  WRITE("#This is a comment\n");
  WRITE("#Manifest File format is such\n");
  WRITE("#<<src>>|<<dest>>|<<link_mode>>\n\n");
  WRITE("#<<src>> and <<dest>> are required to be paths.\n");
  WRITE("#These paths can be either absolute or relative\n");
  WRITE("#(But require them to be relative to the manifest file)\n");
  WRITE("#relative paths do not care where you run the executable\n");
  WRITE("#from as long as it has a manfiest file to build paths off of\n\n");
  WRITE("#<<link_mode>> can be of type `symlink` and more later probably\n\n");
  WRITE("#So in short a example of a line in the manifest is such\n");
  WRITE("#nvim|~/.config/nvim|symlink\n\n");
  WRITE("#if your paths contain | then enclose the path within a quote like\n");
  WRITE("#'~/.config/bobs|special|config'\n");
  WRITE("#both \' and \" are valid quote characters\n");
defer:
  fclose(fd);
  return result;
}
