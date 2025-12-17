#include "build_definitions.h"
#define NOB_IMPLEMENTATION
#include "nob.h"

Nob_Cmd cmd = {0};

// proffesional cross platform developer
#define _WIN32

#ifdef _WIN32
#define RAYLIB_LIB "raylib/windows/libraylib.a"
#else
#define RAYLIB_LIB "raylib/linux/libraylib.a"
#endif

#define generator_append(output_file, format, ...)                             \
  fprintf(output_file, format " /* generated in %s:%d */\n", ##__VA_ARGS__,        \
          __FILE__, __LINE__)

// Appends file data as a c-array to given file.
// Return true on success
bool pack_data(FILE *output_file, char *arr_name, char *data_file) {
  FILE *file = fopen(data_file, "rb");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", data_file);
    fprintf(stderr, "Make sure all resources exist!");
    return false;
  }

  generator_append(output_file, "const unsigned char %s[] = {", arr_name);
  size_t count = 0;
  int byte;
  while ((byte = fgetc(file)) != EOF) {
    fprintf(output_file, "0x%02X,", byte);
    count++;
  }
  fseek(output_file, -1, SEEK_CUR);
  generator_append(output_file, "};");
  generator_append(output_file, "const size_t %s_size = %lu;", arr_name, count);

  fclose(file);
  return true;
}

bool embed_resources() {
  FILE *packed_file = fopen(PACKED_FILE, "wb");
  if (!packed_file) {
    fprintf(stderr, "Couldn't open " PACKED_FILE " for writing\n");
    return false;
  }
  generator_append(packed_file, "#ifndef PACKED_FILE_H");
  generator_append(packed_file, "#define PACKED_FILE_H");
  generator_append(packed_file, "#include <stddef.h>");

  // Pack textures
#define X(arr_name, img_name)                                                  \
  if (!pack_data(packed_file, arr_name, img_name))                             \
    return 1;
  TEXTURE_LIST
#undef X

  // Pack fonts
#define X(arr_name, img_name)                                                  \
  if (!pack_data(packed_file, arr_name, img_name))                             \
    return 1;
    FONT_LIST
#undef X

  generator_append(packed_file, "#endif // PACKED_FILE_H");
  fclose(packed_file);
  return true;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "build_definitions.h");

  if(!embed_resources()) {
    nob_log(NOB_ERROR, "failed generating " PACKED_FILE);
    return 1;
  }
  
  // nob_cc(&cmd);
  nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc");
  nob_cc_flags(&cmd);
  nob_cmd_append(&cmd, "-g", "-O0");
  nob_cmd_append(&cmd, "-Iraylib/include");
  nob_cmd_append(&cmd, "-Iexternal_includes");
  nob_cc_output(&cmd, "src/main");
  nob_cc_inputs(&cmd, "src/main.c");
  nob_cc_inputs(&cmd, "src/game.c");
  nob_cc_inputs(&cmd, "src/communication.c");
  nob_cc_inputs(&cmd, "src/protocol.c");
  nob_cmd_append(&cmd, RAYLIB_LIB);
  nob_cmd_append(&cmd, "-lm");
#ifdef _WIN32
  nob_cmd_append(&cmd, "-lopengl32", "-lgdi32", "-lwinmm", "-lshell32");
#endif
  // nob_cmd_append(&cmd, "-DUI_WORK");

  if (!nob_cmd_run(&cmd))
    return 1;
  return 0;
}
