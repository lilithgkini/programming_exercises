#include "../../include/common.h"
#include "../../include/getops_c.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define HOST "127.0.0.1"

int send_hello(int fd) {
  char buf[1024] = {0};

  proto_hdr_t *hdr = (proto_hdr_t *)buf;
  hdr->state = MSG_HELLO_REQ;
  hdr->len = 1;

  hdr->state = htonl(hdr->state);
  hdr->len = htons(hdr->len);

  proto_hello_req *hello = (proto_hello_req *)&hdr[1];
  hello->version = PROTO_VERSION;
  hello->version = htons(PROTO_VERSION);

  write(fd, buf, sizeof(proto_hdr_t) + sizeof(proto_hello_req));
  printf("Send the hello request\n");

  read(fd, buf, sizeof(buf));
  hdr->state = ntohl(hdr->state);

  if (hdr->state == MSG_ERROR) {
    printf("Failed the Hello protocol\n");
    return STATUS_ERROR;
  } else if (hdr->state == MSG_HELLO_RES) {
    printf("Succsessfully send the hello\n");
  }

  return STATUS_SUCCESS;
}

int get_employees(int fd) {
  char buf[BUFF_SIZE] = {0};
  proto_hdr_t *hdr = (proto_hdr_t *)buf;
  hdr->state = MSG_EMPLOYEE_LIST_REQ;
  hdr->len = 0;

  hdr->state = htonl(hdr->state);
  hdr->len = htonl(hdr->len);

  printf("Requesting to list the employees\n");
  write(fd, buf, sizeof(proto_hdr_t));

  read(fd, buf, sizeof(buf));
  hdr->state = ntohl(hdr->state);

  if (hdr->state == MSG_ERROR) {
    printf("Failed to request to list employees\n");
    return STATUS_ERROR;
  } else if (hdr->state == MSG_EMPLOYEE_LIST_RES) {
    printf("Listing the employees:\n");
    proto_employee_list_res *list_emp = (proto_employee_list_res *)&hdr[1];
    int count = ntohs(hdr->len);
    printf("count: %d\n", count);
    for (int i = 0; i < count; i++) {
      read(fd, list_emp, sizeof(proto_employee_list_res));
      list_emp->hours = ntohl(list_emp->hours);

      printf("\n");
      printf("Employee ID: %d\n", i + 1);
      printf("\t Name: %s\n", list_emp->name);
      printf("\t Address: %s\n", list_emp->address);
      printf("\t Hours: %d\n", list_emp->hours);
      printf("\n");
    }
  }

  return STATUS_SUCCESS;
}
int send_employee(int fd, char *employee_str) {
  // send the request to add the employee
  char buf[BUFF_SIZE] = {0};
  proto_hdr_t *hdr = (proto_hdr_t *)buf;
  hdr->state = MSG_EMPLOYEE_ADD_REQ;
  hdr->len = 1;

  hdr->state = htonl(hdr->state);
  hdr->len = htonl(hdr->len);

  proto_employee_add_req *employee = (proto_employee_add_req *)&hdr[1];
  strncpy(employee->buf, employee_str, sizeof(employee->buf));

  printf("Sending the employee\n");
  write(fd, buf, sizeof(proto_hdr_t) + sizeof(proto_employee_add_req));

  read(fd, buf, sizeof(buf));
  hdr->state = ntohl(hdr->state);

  if (hdr->state == MSG_ERROR) {
    printf("Failed to send employee\n");
    return STATUS_ERROR;
  } else if (hdr->state == MSG_EMPLOYEE_ADD_RES) {
    printf("Successfully send the employee\n");
  }

  return STATUS_SUCCESS;
}

int main(int argc, char **argv) {
  bool list = false;
  char *add_emp;
  struct sockaddr_in addressInfo = {0};
  socklen_t addressLen = sizeof(struct sockaddr_in);
  addressInfo.sin_family = AF_INET;
  addressInfo.sin_addr.s_addr = inet_addr(HOST);
  addressInfo.sin_port = htons(PORT);

  parse_args(argc, argv, &list, &add_emp);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket");
    return -1;
  }
  if (connect(fd, (struct sockaddr *)&addressInfo, addressLen) == -1) {
    perror("connect");
    close(fd);
    return -1;
  }

  // send the hello protocol
  if (send_hello(fd) == STATUS_ERROR) {
    close(fd);
    return -1;
  }

  // add employee
  if (add_emp) {
    printf("Adding an employee %s\n", add_emp);
    send_employee(fd, add_emp);
  }

  if (list) {
    printf("Adding an employee %s\n", add_emp);
    get_employees(fd);
  }
  // closing up
  close(fd);
  return 0;
}
