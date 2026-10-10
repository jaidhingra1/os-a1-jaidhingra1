#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int match(char *s, char *p) {
  if (*p == '\0')
    return *s == '\0';

  if (*p == '*')
    return match(s, p + 1) || (*s != '\0' && match(s + 1, p));

  if (*s != '\0' && *s == *p)
    return match(s + 1, p + 1);

  return 0;
}

int isdot(char *name) {
  if (strcmp(name, ".") == 0)
    return 1;

  if (strcmp(name, "..") == 0)
    return 1;

  return 0;
}


void find(char *path, char *pattern, int type) {
  char buf[512], *p;
  int fd;
  int show;
  struct dirent de;
  struct stat st;

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }
  if (st.type != T_DIR) {
    close(fd);
    return;
  }
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    exit(1);
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p = '/';
  p++;

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = '\0';

    if (isdot(p))
      continue;

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      close(fd);
      exit(1);
    }

    show = 0;

    if (type == 0)
      show = 1;

    if (type == T_DIR && st.type == T_DIR)
      show = 1;

    if (type == T_FILE) {
      if (st.type == T_FILE || st.type == T_DEVICE)
        show = 1;
    }

    if (show && match(p, pattern))
      printf("%s\n", buf);

    if (st.type == T_DIR)
      find(buf, pattern, type);
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  int type;

  type = 0;

  if (argc != 3 && argc != 5) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0) {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if (strcmp(argv[4], "f") == 0) {
      type = T_FILE;
    } else if (strcmp(argv[4], "d") == 0) {
      type = T_DIR;
    } else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  find(argv[1], argv[2], type);
  exit(0);
}
