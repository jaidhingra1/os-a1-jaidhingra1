#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  char *args[MAXARG + 1];
  int mask;
  int i;

  if (argc < 3 || argc - 2 > MAXARG) {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }

  mask = atoi(argv[1]);

  for (i = 2; i < argc; i++)
    args[i - 2] = argv[i];

  args[argc - 2] = 0;

  if (trace(mask) < 0) {
    fprintf(2, "trace: system call failed\n");
    exit(1);
  }

  exec(args[0], args);

  fprintf(2, "trace: exec %s failed\n", args[0]);
  exit(1);
}
