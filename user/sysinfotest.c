#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

void
check(int condition, char *message)
{
  if (condition == 0) {
    fprintf(2, "sysinfotest: %s\n", message);
    exit(1);
  }
}

void
getinfo(struct sysinfo *info)
{
  check(sysinfo(info) == 0, "sysinfo failed");
}

int
main(void)
{
  struct sysinfo before;
  struct sysinfo after;
  char *memory;
  char c;
  int fd;
  int pipes[2];
  int pid;
  int status;

  check(sysinfo((struct sysinfo *)-1) == -1,
        "invalid address was accepted");
  printf("invalid address: OK\n");

  /* Allocating memory must reduce free memory. */
  getinfo(&before);

  memory = sbrk(4096);
  check(memory != SBRK_ERROR, "memory allocation failed");

  memset(memory, 0, 4096);

  getinfo(&after);
  check(after.freemem < before.freemem,
        "free memory did not decrease");

  memory = sbrk(-4096);
  check(memory != SBRK_ERROR, "memory release failed");

  getinfo(&before);
  check(before.freemem > after.freemem,
        "free memory did not increase");

  printf("memory allocation and release: OK\n");

  /* Opening README adds one file-table entry. */
  getinfo(&before);

  fd = open("README", O_RDONLY);
  check(fd >= 0, "open failed");

  getinfo(&after);
  check(after.nopenfile == before.nopenfile + 1,
        "open-file count did not increase");

  check(close(fd) == 0, "close failed");

  getinfo(&after);
  check(after.nopenfile == before.nopenfile,
        "open-file count did not return to normal");

  printf("open and close: OK\n");

  /* A pipe adds two file-table entries. */
  getinfo(&before);

  check(pipe(pipes) == 0, "pipe failed");

  getinfo(&after);
  check(after.nopenfile == before.nopenfile + 2,
        "pipe did not add two open files");

  check(close(pipes[0]) == 0, "pipe read-end close failed");
  check(close(pipes[1]) == 0, "pipe write-end close failed");

  getinfo(&after);
  check(after.nopenfile == before.nopenfile,
        "pipe close did not restore the file count");

  printf("pipe and close: OK\n");

  /*
   * Keep the child alive with a pipe while the parent
   * checks the process count.
   */
  check(pipe(pipes) == 0, "process-test pipe failed");
  getinfo(&before);

  pid = fork();
  check(pid >= 0, "fork failed");

  if (pid == 0) {
    close(pipes[1]);

    if (read(pipes[0], &c, 1) != 1)
      exit(1);

    close(pipes[0]);
    exit(0);
  }

  close(pipes[0]);

  getinfo(&after);
  check(after.nproc == before.nproc + 1,
        "fork did not increase the process count");

  c = 'x';
  check(write(pipes[1], &c, 1) == 1,
        "could not signal the child");
  close(pipes[1]);

  check(wait(&status) == pid, "wait failed");
  check(status == 0, "child failed");

  getinfo(&after);
  check(after.nproc == before.nproc,
        "process count did not return to normal");

  printf("fork and wait: OK\n");
  printf("sysinfotest: ALL TESTS PASSED\n");

  exit(0);
}
