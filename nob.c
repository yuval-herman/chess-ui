#define NOB_IMPLEMENTATION
#include "nob.h"

Nob_Cmd cmd = {0};

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  nob_cmd_append(&cmd, "-g", "-O0");
  nob_cc_output(&cmd, "main");
  nob_cc_inputs(&cmd, "main.c");
  nob_cc_inputs(&cmd, "game.c");
  nob_cmd_append(&cmd, "-lm", "-lraylib");

  if (!nob_cmd_run(&cmd))
    return 1;
  return 0;
}
