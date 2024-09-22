#include "../../include/parse.h"
#include "../../include/common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int get_header(int fd, bool newfile, struct dbheader_t **dbhdr) {
  struct dbheader_t *headers = calloc(1, sizeof(struct dbheader_t));
  if (headers == (void *)-1) {
    perror("calloc");
    printf("Failed to allocate memory for db headers\n");
    return -1;
  }

  if (newfile == true) {

    headers->magic = MAGIC_VALUE;
    headers->version = 0x1;
    headers->count = 0;
    headers->filesize = sizeof(struct dbheader_t);

    *dbhdr = headers;
    return 0;

  } else {
    if (read(fd, headers, sizeof(struct dbheader_t)) !=
        sizeof(struct dbheader_t)) {
      printf("Failed to validate the headers\n");
      perror("read");
      free(headers);
      return -1;
    }

    headers->magic = ntohl(headers->magic);
    headers->version = ntohs(headers->version);
    headers->count = ntohs(headers->count);
    headers->filesize = ntohl(headers->filesize);

    if (headers->magic != MAGIC_VALUE) {
      printf("Wrong magic value\n");
      free(headers);
      return -1;
    }
    if (headers->version != 1) {
      printf("Wrong version\n");
      free(headers);
      return -1;
    }
    *dbhdr = headers;
    return 0;
  }
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  int count = dbhdr->count;
  if (fd == -1) {
    printf("Wrong FD for the read employees\n");
    return -1;
  }

  struct employee_t *employees =
      calloc(count, count * sizeof(struct employee_t));
  if (employees == NULL) {
    perror("calloc");
    printf("Failed to allocate memory for employees\n");
    return -1;
  }

  if (read(fd, employees, count * sizeof(struct employee_t)) == -1) {
    printf("Failed to read the employees from the file\n");
    perror("read");
    free(employees);
    return -1;
  }

  for (int i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;
  return 0;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employeeptr,
                 char *employee_str) {
  int count = dbhdr->count;
  char *name = strtok(employee_str, ",");
  if (!name) {
    return STATUS_ERROR;
  }
  char *address = strtok(NULL, ",");
  if (!address) {
    return STATUS_ERROR;
  }
  char *hours_str = strtok(NULL, ",");
  if (!hours_str) {
    return STATUS_ERROR;
  }
  unsigned short hours = atoi(hours_str);
  if (hours == 0) {
    return STATUS_ERROR;
  }

  dbhdr->count++;
  *employeeptr =
      realloc(*employeeptr, dbhdr->count * sizeof(struct employee_t));
  if (employeeptr == NULL) {
    printf("Failed to realloc memory for the aditional employee\n");
    return STATUS_ERROR;
  }
  struct employee_t *employee = *employeeptr;

  strncpy(employee[count].name, name, sizeof(employee[count].name));
  strncpy(employee[count].address, address, sizeof(employee[count].address));
  employee[count].hours = hours;

  return STATUS_SUCCESS;
}

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

  for (int i = 0; i < dbhdr->count; i++) {
    printf("\n");
    printf("Employee ID: %d\n", i);
    printf("\t Name: %s\n", employees[i].name);
    printf("\t Address: %s\n", employees[i].address);
    printf("\t Hours: %d\n", employees[i].hours);
    printf("\n");
  }
}
