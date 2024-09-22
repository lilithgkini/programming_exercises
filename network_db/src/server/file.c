#include "../../include/parse.h"
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int get_db_file(char *path, bool *newfile) {
  if (*newfile == false) {
    printf("Opening existing file..\n");
    int fd = open(path, O_RDWR);
    if (fd == -1) {
      perror("open");
      return -1;
    }
    return fd;
  } else {
    printf("-n flag was provided\n");
    printf("Creating it..\n");
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
      perror("open");
      return -1;
    }
    return fd;
  }
}

int write_db_file(int fd, struct dbheader_t *dbhdr,
                  struct employee_t *employees) {
  if (fd == -1) {
    printf("Wrong FD for the write\n");
    return -1;
  }
  unsigned short realcount = dbhdr->count;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->version = htons(dbhdr->version);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) +
                          (realcount * sizeof(struct employee_t)));

  lseek(fd, 0, SEEK_SET);
  if (write(fd, dbhdr, sizeof(struct dbheader_t)) == -1) {
    printf("failed to write the headers to the file\n");
    perror("write");
    return -1;
  }

  if (employees == NULL) {
    printf("No Employees to write\n");
    return -1;
  }
  for (int i = 0; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    if (write(fd, &employees[i], sizeof(struct employee_t)) == -1) {
      printf("failed to write the employees to the file\n");
      perror("write");
      return -1;
    }
    employees[i].hours = ntohl(employees[i].hours);
  }

  dbhdr->magic = ntohl(dbhdr->magic);
  dbhdr->version = ntohs(dbhdr->version);
  dbhdr->count = ntohs(dbhdr->count);
  dbhdr->filesize = ntohl(dbhdr->filesize);
  return 0;
}
