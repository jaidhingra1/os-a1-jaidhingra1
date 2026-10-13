#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

void check(int condition, char *message) {
  if (condition == 0) {
    fprintf(2, "getprocstest: %s\n", message);
    exit(1);
  }
}

int main(void) {
  static struct uproc processes[NPROC];
  struct uproc small[2];
  int count;
  int found;
  int pid;
  int status;
  int i;
  int j;
  int k;

  check(getprocs(processes, -1) == -1,
        "negative max was accepted");

  check(getprocs(processes, 0) == 0,
        "zero max did not return zero");

  check(getprocs((struct uproc *)-1, 1) == -1,
        "invalid address was accepted");

  printf("invalid arguments and zero max: OK\n");

  memset(small, 0, sizeof(small));
  small[1].pid = -1234;

  check(getprocs(small, 1) == 1,
        "max of one did not return one entry");

  check(small[1].pid == -1234,
        "copied beyond the requested limit");

  count = getprocs(processes, NPROC);
  check(count > 0 && count <= NPROC,
        "incorrect process count");

  check(small[0].pid == processes[0].pid,
        "entries are not in process-table order");

  found = 0;

  for (i = 0; i < count; i++) {
    if (processes[i].pid == getpid()) {
      found = 1;
      check(processes[i].state == 4,
            "current process is not running");
      check(processes[i].ppid > 0,
            "current process has no parent");
      check(processes[i].sz > 0,
            "current process has no memory size");
      check(strcmp(processes[i].name, "getprocstest") == 0,
            "incorrect process name");
    }
  }

  check(found, "current process was not listed");

  printf("entry limit and process fields: OK\n");

  /* Read the table repeatedly while children run and exit. */
  for (i = 0; i < 20; i++) {
    pid = fork();
    check(pid >= 0, "fork failed");

    if (pid == 0) {
      pause(1);
      exit(0);
    }

    for (j = 0; j < 10; j++) {
      count = getprocs(processes, NPROC);
      check(count > 0 && count <= NPROC,
            "process listing failed during fork/exit");

      for (k = 0; k < count; k++) {
        check(processes[k].pid > 0, "invalid process ID");
        check(processes[k].state >= 1 &&
              processes[k].state <= 5,
              "invalid process state");
        check(processes[k].name[15] == '\0',
              "process name is not terminated");
      }
    }

    check(wait(&status) == pid, "wait failed");
    check(status == 0, "child failed");
  }

  printf("repeated fork/exit: OK\n");
  printf("getprocstest: ALL TESTS PASSED\n");
  exit(0);
}
