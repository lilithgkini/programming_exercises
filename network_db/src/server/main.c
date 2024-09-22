#include "../../include/file.h"
#include "../../include/common.h"
#include "../../include/getops.h"
#include "../../include/parse.h"
#include "../../include/server_socket.h"
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
  bool newfile = false;
  char *filepath = NULL;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  if(parse_args(argc, argv, &newfile, &filepath) == STATUS_ERROR){
    return -1;
  }

  if (filepath == NULL) {
    get_help(argv);
  }

  int fd = get_db_file(filepath, &newfile);
  if (fd == -1) {
    printf("Failed to open db\n");
    return -1;
  }

  if (get_header(fd, newfile, &dbhdr) == -1) {
    printf("Couldnt get headers\n");
    close(fd);
  }

  if (dbhdr == NULL) {
    printf("Headers are NULL..\n");
    free(dbhdr);
    close(fd);
    return -1;
  }

  if (read_employees(fd, dbhdr, &employees) == -1) {
    printf("Failed to read employees\n");
    free(employees);
    free(dbhdr);
    close(fd);
    return -1;
  }

  int socket_fd = create_socket(PORT);
  if(socket_fd == STATUS_ERROR){
    printf("Failed to create a socket\n");
    free(employees);
    free(dbhdr);
    close(fd);
    return -1;
  }
  poll_loop(fd, socket_fd, dbhdr, &employees);

  free(employees);
  free(dbhdr);
  close(fd);
  close(socket_fd);
  return 0;
}
