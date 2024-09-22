#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "common.h"
#include "parse.h"

#define MAX_CLIENTS 3

typedef enum {
  NEW,
  HELLO,
  DISCONNECTED,
  STATE,
} state_e;

typedef struct {
  int fd;
  state_e state;
  char buf[BUFF_SIZE];
} client_t;

int create_socket(int);
int handle_client(int ,struct dbheader_t *, struct employee_t **, client_t*);
int poll_loop(int, int, struct dbheader_t *, struct employee_t **);
void err_res(client_t*, proto_hdr_t *);
void hello_res(client_t*, proto_hdr_t *);
void add_emp_res(client_t*, proto_hdr_t *);

#endif
