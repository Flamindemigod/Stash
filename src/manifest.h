#include "opts.h"
#include <stdbool.h>
#include <stdio.h>
enum LINKMODE { SYMLINK };

typedef struct {
  const char *src;
  const char *dest;
  enum LINKMODE mode;
} ManifestField;

typedef struct {
  ManifestField *items;
  size_t count;
  size_t capacity;
} Manifest;

bool parse_manifest(Opts *opts, Manifest *manifest);
bool generate_example_manifest(Opts *opts);
void free_manifest(Manifest *manifest);
