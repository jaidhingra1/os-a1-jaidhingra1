#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[]) {
    if (argc != 2 || argv[1][0] == '\0') {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  for (int i = 0; argv[1][i] != '\0'; i++) {
    if (argv[1][i] < '0' || argv[1][i] > '9') {
      fprintf(2, "usage: sleep <ticks>\n");
      exit(1);
    }
  }

  int ticks = atoi (argv[1]);

  pause (ticks);

  exit (0);
}
