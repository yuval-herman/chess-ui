#include <stdio.h>
#include <stdlib.h>
#define NOB_WARN_DEPRECATED
#define NOB_EXPERIMENTAL_DELETE_OLD
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "build_definitions.h"

#ifdef _WIN32
#define RAYLIB_LIB "raylib/windows/libraylib.a"
#else
#define RAYLIB_LIB "raylib/linux/libraylib.a"
#endif

#define BUILD_CACHE ".build_cache/"

#define generator_append(output_file, format, ...)                             \
  fprintf(output_file, format " /* generated in %s:%d */\n", ##__VA_ARGS__,    \
          __FILE__, __LINE__)

// nob_cmd_append(cmd, "-O0", "-g", "-fsanitize=address,undefined");
// nob_cmd_append(cmd, "-O2", "-march=native");

#define project_flags(cmd)                                                               \
  do {                                                                                   \
    nob_cmd_append(cmd, "-O0", "-g", "-fsanitize=address,undefined");                    \
    nob_cmd_append(cmd, "-Iraylib/include", "-Iexternal_includes");                      \
  } while (0)

// Appends file data as a c-array to given file.
// Return true on success
bool pack_data(FILE *output_file, FILE *header_file, char *arr_name, char *data_file) {
  FILE *file = fopen(data_file, "rb");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", data_file);
    fprintf(stderr, "Make sure all resources exist!");
    return false;
  }

  size_t mark = nob_temp_save(); {
    char *decl = nob_temp_sprintf("const unsigned char %s[]", arr_name);
    generator_append(header_file, "extern %s;", decl);
    generator_append(output_file, "%s = {", decl);
    size_t count = 0;
    int byte;
    while ((byte = fgetc(file)) != EOF) {
      fprintf(output_file, "0x%02X,", byte);
      count++;
    }
    fseek(output_file, -1, SEEK_CUR);
    generator_append(output_file, "};");
    decl = nob_temp_sprintf("const int %s_size", arr_name);
    generator_append(header_file, "extern %s;", decl);
    // Raylib expects int in LoadFromFile* functions
    generator_append(output_file, "%s = %lu;", decl, count);
  }
  nob_temp_rewind(mark);

  fclose(file);
  return true;
}

bool embed_resources() {
  FILE *packed_file_header = fopen(PACKED_FILE_HEADER, "wb");
  if (!packed_file_header) {
    fprintf(stderr, "Couldn't open " PACKED_FILE_HEADER " for writing\n");
    return false;
  }
  FILE *packed_file_source = fopen(PACKED_FILE_SOURCE, "wb");
  if (!packed_file_source) {
    fprintf(stderr, "Couldn't open " PACKED_FILE_SOURCE " for writing\n");
    return false;
  }
  generator_append(packed_file_source, "#include \"packed_files.h\"");
  generator_append(packed_file_header, "#ifndef PACKED_FILE_H");
  generator_append(packed_file_header, "#define PACKED_FILE_H");

  // Pack textures
#define X(arr_name, img_name)                                                                      \
  if (!pack_data(packed_file_source, packed_file_header, arr_name, img_name)) return 1;
  TEXTURE_LIST
#undef X

  // Pack fonts
#define X(arr_name, img_name)                                                                      \
  if (!pack_data(packed_file_source, packed_file_header, arr_name, img_name)) return 1;
  FONT_LIST
#undef X

  generator_append(packed_file_header, "#endif // PACKED_FILE_H");
  fclose(packed_file_header);
  return true;
}

char *build_file(char *file_name, bool force) {
  const char *base_file = nob_path_name(file_name);
  char *o_file_path =
      nob_temp_sprintf("%s%.*s.o", BUILD_CACHE, (int)strlen(base_file) - 2, base_file);
  if (force || nob_needs_rebuild1(o_file_path, file_name)) {
    size_t mark = nob_temp_save(); {
      Nob_Cmd cmd = {0};
      nob_cc(&cmd);
      nob_cc_flags(&cmd);
      project_flags(&cmd);
      nob_cmd_append(&cmd, "-c");
      nob_cc_output(&cmd, o_file_path);
      nob_cc_inputs(&cmd, file_name);
      nob_cmd_append(&cmd, "-Iraylib/include", "-Iexternal_includes");
      nob_cmd_run(&cmd);
    }
    nob_temp_rewind(mark);
  } else nob_log(NOB_INFO, "Skeeping %s rebuild", file_name);
  return o_file_path;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "build_definitions.h");

  if(!nob_mkdir_if_not_exists(BUILD_CACHE)) return 1;

  nob_log(NOB_INFO, "packing resources into " PACKED_FILE_SOURCE);
  if (!embed_resources()) {
    nob_log(NOB_ERROR, "failed generating " PACKED_FILE_SOURCE);
    return 1;
  }
  bool force = argc > 1;
  Nob_Cmd cmd = {0};
  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  nob_cc_output(&cmd, "main");
  project_flags(&cmd);
  nob_cc_inputs(&cmd, PACKED_FILE_SOURCE);
  nob_cc_inputs(&cmd, build_file("src/main.c", force));
  nob_cc_inputs(&cmd, build_file("src/game.c", force));
  nob_cc_inputs(&cmd, build_file("src/layout.c", force));
#ifdef _WIN32
  nob_cc_inputs(&cmd, build_file("src/communication_windows.c", force));
#else
  nob_cc_inputs(&cmd, build_file("src/communication_posix.c", force));
#endif
  nob_cc_inputs(&cmd, build_file("src/protocol.c", force));
  nob_cc_inputs(&cmd, build_file("src/rules.c", force));
  nob_cmd_append(&cmd, RAYLIB_LIB);
  nob_cmd_append(&cmd, "-lm");
#ifdef _WIN32
  nob_cmd_append(&cmd, "-lopengl32", "-lgdi32", "-lwinmm", "-lshell32");
#endif
  // nob_cmd_append(&cmd, "-DUI_WORK");
  nob_cmd_append(&cmd, "-DTESTER_MODE");

  if (!nob_cmd_run(&cmd)) return 1;
  return 0;
}
