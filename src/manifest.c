#include "manifest.h"
#include "nob.h"
#include <stdio.h>
#include <unistd.h>

#define WRITE(str) fputs(str, fd);
#define PARSE_FIELD(sv_out)                                                    \
  do {                                                                         \
    if (sv_seg.data[0] == '\'') {                                              \
      sv_seg.data++;                                                           \
      sv_seg.count--;                                                          \
      sv_out = nob_sv_trim(nob_sv_chop_by_delim(&sv_seg, '\''));               \
      nob_sv_chop_by_delim(&sv_seg, '|');                                      \
    } else if (sv_seg.data[0] == '\"') {                                       \
      sv_seg.data++;                                                           \
      sv_seg.count--;                                                          \
      sv_out = nob_sv_trim(nob_sv_chop_by_delim(&sv_seg, '\"'));               \
      nob_sv_chop_by_delim(&sv_seg, '|');                                      \
    } else {                                                                   \
      sv_out = nob_sv_chop_by_delim(&sv_seg, '|');                             \
    }                                                                          \
    sv_seg = nob_sv_trim(sv_seg);                                              \
  } while (0);

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

bool parse_manifest(Opts *opts, Manifest *manifest) {
  bool result = true;
  size_t line_no = 0;
  Nob_String_Builder sb = {0};
  Nob_String_View sv, sv_seg, src, dest, mode;
  enum LINKMODE link_mode;
  nob_log(NOB_INFO, "Parsing manifest at %s", opts->manifest);
  nob_read_entire_file(opts->manifest, &sb);
  sv = nob_sv_from_parts(sb.items, sb.count);
  sv = nob_sv_trim(sv);
  while (sv.count > 0) {
    sv_seg = nob_sv_trim(nob_sv_chop_by_delim(&sv, '\n'));
    line_no++;
    if (sv_seg.data[0] == '#' || sv_seg.data[0] == '\n') {
      continue;
    }
    PARSE_FIELD(src);
    if (src.count <= 0) {
      nob_log(NOB_ERROR, "%s:%zu: Failed to parse src field", opts->manifest,
              line_no);
      nob_return_defer(false);
    }
    PARSE_FIELD(dest);
    if (dest.count <= 0) {
      nob_log(NOB_ERROR, "%s:%zu: Failed to parse dest filed", opts->manifest,
              line_no);
      nob_return_defer(false);
    }
    PARSE_FIELD(mode);
    if (mode.count <= 0) {
      nob_log(NOB_ERROR, "%s:%zu: Failed to parse mode field", opts->manifest,
              line_no);
      nob_return_defer(false);
    } else {
      if (nob_sv_eq(mode, nob_sv_from_cstr("symlink"))) {
        link_mode = SYMLINK;
      }
    }
    nob_da_append(manifest,
                  ((ManifestField){nob_temp_sv_to_cstr(src),
                                   nob_temp_sv_to_cstr(dest), link_mode}));
  }
defer:
  for (size_t i = 0; i < manifest->count; i++) {
    printf("Manifest field \n\tSrc: %s\n\tDest: %s\n\tMode: %d\n",
           manifest->items[i].src, manifest->items[i].dest,
           manifest->items[i].mode);
  }
  nob_sb_free(sb);
  return result;
}
