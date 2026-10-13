#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

int
main(void)
{
  static struct uproc processes[NPROC];
  char *state;
  int count;
  int i;

  count = getprocs(processes, NPROC);

  if (count < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  for (i = 0; i < count; i++) {
    switch (processes[i].state) {
    case 1:
      state = "used";
      break;
    case 2:
      state = "sleeping";
      break;
    case 3:
      state = "runnable";
      break;
    case 4:
      state = "running";
      break;
    case 5:
      state = "zombie";
      break;
    default:
      state = "unknown";
      break;
    }

    printf("%d\t%d\t%s\t%lu\t%s\n",
           processes[i].pid, processes[i].ppid,
           state, processes[i].sz, processes[i].name);
  }

  exit(0);
}
