#include "manifest.h"
#include "nob.h"
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

char *resolveHome(const char *path) {
  char *result = NULL;
  Nob_String_Builder sb = {0};
  size_t path_len = strlen(path);
  if (path_len <= 0) {
    nob_return_defer(NULL);
  }
  if (path[0] == '~') {
    const char *home = getenv("HOME");
    nob_sb_append_cstr(&sb, home);
    path++;
    path_len--;
  }
  nob_sb_append_buf(&sb, path, path_len);
  nob_return_defer(
      strdup(nob_temp_sv_to_cstr(nob_sv_from_parts(sb.items, sb.count))));
defer:
  nob_sb_free(sb);
  return result;
}

char *get_realpath(Opts *opts, const char *path) {
  char *result = NULL;
  Nob_String_Builder sb = {0};
  char *dir_path = NULL, *manifest_dir = NULL, *path_out = NULL;
  size_t path_len = strlen(path);
  if (path_len <= 0) {
    nob_return_defer(NULL);
  }
  if (path[0] != '~') {
    manifest_dir = dirname(opts->manifest);
    nob_sb_append_cstr(&sb, manifest_dir);
    nob_sb_append_cstr(&sb, "/");
  } else {
    const char *home = getenv("HOME");
    nob_sb_append_cstr(&sb, home);
    path++;
    path_len--;
  }
  nob_sb_append_buf(&sb, path, path_len);
  path = nob_temp_sv_to_cstr(nob_sv_from_parts(sb.items, sb.count));
  dir_path = dirname(strdup((char *)path));
  sb.count = 0;
  nob_sb_append_cstr(&sb, dir_path);
  path_out = (char *)nob_temp_sv_to_cstr(nob_sv_from_parts(sb.items, sb.count));
  path_out = realpath(path_out, 0);
  if (path_out == NULL) {
    char *errStr = strerror(errno);
    nob_log(NOB_ERROR, "Failed to build realpath from path %s : %s", path_out,
            errStr);
    nob_return_defer(NULL);
  }
  sb.count = 0;
  nob_sb_append_cstr(&sb, path_out);
  nob_sb_append_cstr(&sb, "/");
  nob_sb_append_cstr(&sb, basename((char *)path));
  nob_return_defer(
      strdup(nob_temp_sv_to_cstr(nob_sv_from_parts(sb.items, sb.count))));
defer:
  if (path_out != NULL)
    free(path_out);
  if (dir_path != NULL)
    free(dir_path);
  nob_sb_free(sb);
  return result;
}

void free_manifest(Manifest *manifest) { nob_da_free(*manifest); }

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

typedef bool (*Linker)(bool force, enum LINKMODE mode, const char *from,
                       const char *to);
bool dryLinker(bool force, enum LINKMODE mode, const char *from,
               const char *to) {
  bool result = true;
  char *dest = NULL;
  dest = resolveHome(to);
  if (dest == NULL) {
    nob_return_defer(false);
  }
  if (nob_file_exists(dest)) {
    if (force) {
      nob_log(NOB_WARNING, "%s already exists. Removing", dest);
    } else {
      nob_log(NOB_WARNING, "%s already exists. Skipping", dest);
      nob_return_defer(false);
    }
  }
  nob_log(NOB_INFO, "Linking %s => %s", from, dest);
defer:
  free(dest);
  return result;
}
bool trueLinker(bool force, enum LINKMODE mode, const char *from,
                const char *to) {
  bool result = true;
  char *dest = NULL;
  dest = resolveHome(to);
  if (dest == NULL) {
    nob_return_defer(false);
  }
  if (nob_file_exists(dest)) {
    if (force) {
      nob_log(NOB_WARNING, "%s already exists. Removing", dest);
      remove(dest);
    } else {
      nob_log(NOB_WARNING, "%s already exists. Skipping", dest);
      nob_return_defer(false);
    }
  }
  if (!nob_mkdir_if_not_exists(dirname(nob_temp_strdup(dest)))) {
    nob_log(NOB_ERROR, "Failed to build dirs for %s", dest);
    nob_return_defer(false);
  };
  nob_log(NOB_INFO, "Linking %s => %s", from, dest);
  symlink(from, dest);
defer:
  free(dest);
  return result;
}

bool link_manifest(Opts *opts, Manifest *manifest) {
  bool result = true;
  char *src = NULL;
  Linker linker;
  ManifestField *field;
  if (opts->dryRun) {
    linker = dryLinker;
    nob_log(NOB_INFO,
            "Using dry run linker to link manifest. No files will be linked");
  } else {
    linker = trueLinker;
    nob_log(NOB_INFO,
            "Using true linker to link manifest. Files will be linked");
  }
  for (size_t i = 0; i < manifest->count; i++) {
    field = &manifest->items[i];
    src = get_realpath(opts, field->src);
    if (src == NULL) {
      nob_log(NOB_ERROR, "Failed to build src realpath for %s", field->src);
      nob_return_defer(false);
    }
    linker(opts->forceReplace, field->mode, src, field->dest);
  }
defer:
  if (src != NULL)
    free(src);
  return result;
}
