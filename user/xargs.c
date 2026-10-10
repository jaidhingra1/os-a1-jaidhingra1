#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void usage(void) {
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

int run(char *args[]) {
  int pid;
  int status;

  pid = fork();

  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  }

  if (wait(&status) < 0) {
    fprintf(2, "xargs: wait failed\n");
    exit(1);
  }

  return status;
}

int main(int argc, char *argv[]) {
  static char buf[4096];
  char *args[MAXARG + 1];
  char c;
  int first;
  int limit;
  int base;
  int count;
  int used;
  int inword;
  int words;
  int failed;
  int i;
  int n;

  first = 1;
  limit = 0;
  failed = 0;

  if (argc < 2)
    usage();

  if (strcmp(argv[1], "-n") == 0) {
    if (argc < 4)
      usage();

    if (argv[2][0] == '\0')
      usage();

    for (i = 0; argv[2][i] != '\0'; i++) {
      if (argv[2][i] < '0' || argv[2][i] > '9')
        usage();

      /*
       * A batch cannot exceed MAXARG anyway.
       * Stop growing the number to avoid integer overflow.
       */
      if (limit <= MAXARG) {
        limit = limit * 10 + (argv[2][i] - '0');

        if (limit > MAXARG)
          limit = MAXARG + 1;
      }
    }

    if (limit == 0)
      usage();

    first = 3;
  }

  base = argc - first;

  if (base > MAXARG)
    usage();

  for (i = 0; i < base; i++)
    args[i] = argv[first + i];

  count = base;
  used = 0;
  inword = 0;
  words = 0;

  while (1) {
    n = read(0, &c, 1);

    if (n < 0) {
      fprintf(2, "xargs: read failed\n");
      exit(1);
    }

    if (n == 0)
      break;

    if (c == ' ' || c == '\t' || c == '\n') {
      if (inword) {
        buf[used] = '\0';
        used++;
        inword = 0;
        words++;
      }

      if ((limit > 0 && words == limit) ||
          (limit == 0 && c == '\n' && words > 0)) {
        args[count] = 0;

        if (run(args) != 0)
          failed = 1;

        count = base;
        used = 0;
        inword = 0;
        words = 0;
      }
    } else {
      if (used >= sizeof(buf) - 1) {
        fprintf(2, "xargs: input too long\n");
        exit(1);
      }

      if (inword == 0) {
        if (count >= MAXARG) {
          fprintf(2, "xargs: too many arguments\n");
          exit(1);
        }

        args[count] = &buf[used];
        count++;
        inword = 1;
      }

      buf[used] = c;
      used++;
    }
  }

  /* Finish a last word even if input has no ending newline. */
  if (inword) {
    buf[used] = '\0';
    words++;
  }

  /* Run any words left at the end of input. */
  if (words > 0) {
    args[count] = 0;

    if (run(args) != 0)
      failed = 1;
  }

  exit(failed);
}
