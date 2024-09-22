#include "../../include/server_socket.h"
#include "../../include/common.h"
#include "../../include/file.h"
#include "../../include/parse.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

client_t clients[MAX_CLIENTS];

// Handling protocol responses to the client
void err_res(client_t *client, proto_hdr_t *hdr) {
  hdr->state = htonl(MSG_ERROR);
  hdr->len = htons(0);

  write(client->fd, hdr, sizeof(proto_hdr_t));
}

void hello_res(client_t *client, proto_hdr_t *hdr) {
  hdr->state = htonl(MSG_HELLO_RES);
  hdr->len = htons(1);

  proto_hello_res *hello = (proto_hello_res *)&hdr[1];
  hello->state = htonl(PROTO_VERSION);

  write(client->fd, hdr, sizeof(proto_hdr_t));
}

void add_emp_res(client_t *client, proto_hdr_t *hdr) {
  hdr->state = htonl(MSG_EMPLOYEE_ADD_RES);
  hdr->len = htons(0);

  write(client->fd, hdr, sizeof(proto_hdr_t));
}

void list_emp_res(client_t *client, struct dbheader_t *dbhdr,
                  struct employee_t **employeeptr) {
  proto_hdr_t *hdr = (proto_hdr_t *)client->buf;
  int count = dbhdr->count;

  hdr->state = htonl(MSG_EMPLOYEE_LIST_RES);
  hdr->len = htons(count);

  write(client->fd, hdr, sizeof(proto_hdr_t));

  proto_employee_list_res *list_emp = (proto_employee_list_res *)&hdr[1];
  struct employee_t *employees = *employeeptr;

  for (int i = 0; i < count; i++) {

    strncpy(list_emp->name, employees[i].name, sizeof(list_emp->name));
    strncpy(list_emp->address, employees[i].address, sizeof(list_emp->address));
    list_emp->hours = employees[i].hours;
    list_emp->hours = htonl(list_emp->hours);

    write(client->fd, list_emp, sizeof(proto_employee_list_res));
  }
}

// Handle the clients
int init_clients() {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    clients[i].fd = -1;
    clients[i].state = NEW;
    memset(clients[i].buf, 0, BUFF_SIZE);
  }
  return STATUS_SUCCESS;
}

int find_free_index() {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].fd == -1) {
      return i;
    }
  }
  return STATUS_ERROR;
}

int find_fd_slot(int fd) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].fd == fd) {
      return i;
    }
  }
  return STATUS_ERROR;
}

int poll_loop(int dbfd, int socket_fd, struct dbheader_t *dbhdr,
              struct employee_t **employees) {
  int nfds, conn_fd, cfd, slot, free_index;
  struct pollfd fds[MAX_CLIENTS + 1] = {0};
  init_clients();
  struct sockaddr_in clientInfo = {0};
  socklen_t clientLen = sizeof(struct sockaddr_in);

  fds[0].fd = socket_fd;
  fds[0].events = POLLIN;
  nfds = 1;
  while (1) {

    int ii = 1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd != 1) {
        fds[ii].fd = clients[i].fd;
        fds[ii].events = POLLIN;
        ii++;
      }
    }
    int ready_poll_number = poll(fds, nfds, -1);
    if (ready_poll_number <= 0) {
      perror("poll");
      return STATUS_ERROR;
    }

    if (fds[0].revents & POLLIN) {
      conn_fd = accept(socket_fd, (struct sockaddr *)&clientInfo,
                       (socklen_t *)&clientLen);
      if (conn_fd == -1) {
        perror("poll");
        return STATUS_ERROR;
      }
      printf("New connection by %s on port %d\n",
             inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));
      free_index = find_free_index();
      if (free_index == STATUS_ERROR) {
        printf("Server is full\n");
        close(conn_fd);
        continue;
      }
      clients[free_index].fd = conn_fd;
      clients[free_index].state = HELLO;
      nfds++;
      ready_poll_number--;
    }
    for (int i = 0; (i < nfds) && (ready_poll_number > 0); i++) {
      if (fds[i].fd & POLLIN) {
        ready_poll_number--;
        cfd = fds[i].fd;
        slot = find_fd_slot(cfd);
        int byte_read = read(cfd, &clients[slot].buf, BUFF_SIZE);
        if (byte_read <= 0) {
          printf("Client disconnected\n");
          close(cfd);
          clients[slot].fd = -1;
          clients[slot].state = DISCONNECTED;
          memset(&clients[slot].buf, 0, BUFF_SIZE);
          nfds--;
        } else {
          printf("Recieved data from the client\n");
          int status = handle_client(dbfd, dbhdr, employees, &clients[slot]);
        }
      }
    }
  }

  return STATUS_SUCCESS;
}

int handle_client(int dbfd, struct dbheader_t *dbhdr,
                  struct employee_t **employees, client_t *client) {
  proto_hdr_t *hdr = (proto_hdr_t *)client->buf;

  hdr->state = ntohl(hdr->state);
  hdr->len = ntohs(hdr->len);

  if (client->state == HELLO) {
    if ((hdr->state != MSG_HELLO_REQ) || (hdr->len != 1)) {
      printf("Didnt get msg hello request\n");
      err_res(client, hdr);
      return STATUS_ERROR;
    }
    proto_hello_req *hello = (proto_hello_req *)&hdr[1];
    hello->version = ntohs(hello->version);
    if (hello->version != PROTO_VERSION) {
      printf("Wrong protocol version\n");
      err_res(client, hdr);
      return STATUS_ERROR;
    }
    hello_res(client, hdr);
    client->state = STATE;

    printf("Changed client state to STATE\n");
  }

  if (client->state == STATE) {
    // TODO: Handle the data requests (add employee, list, etc)
    if ((hdr->state == MSG_EMPLOYEE_ADD_REQ) && (hdr->len = 1)) {
      proto_employee_add_req *add_emp = (proto_employee_add_req *)&hdr[1];

      // adding employee functionality
      if (add_employee(dbhdr, employees, add_emp->buf) == STATUS_ERROR) {
        err_res(client, hdr);
        return STATUS_ERROR;
      } else {

        add_emp_res(client, hdr);

        if (write_db_file(dbfd, dbhdr, *employees) == -1) {
          printf("Failed to write\n");
          return STATUS_ERROR;
        }
      }
    }
    if ((hdr->state == MSG_EMPLOYEE_LIST_REQ) && (hdr->len = 1)) {
      proto_employee_add_req *list_emp = (proto_employee_add_req *)&hdr[1];
      // TODO: List employees to the client by sending the details of each
      // employee from our struct
      list_emp_res(client, dbhdr, employees);
    }
  }

  return STATUS_SUCCESS;
}

int create_socket(int port) {
  struct sockaddr_in addressInfo = {0};
  addressInfo.sin_family = AF_INET;
  addressInfo.sin_addr.s_addr = htonl(INADDR_ANY);
  addressInfo.sin_port = htons(port);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket");
    return STATUS_ERROR;
  }

  int optval = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    close(fd);
    return STATUS_ERROR;
  }

  if (bind(fd, (struct sockaddr *)&addressInfo, sizeof(addressInfo)) == -1) {
    perror("bind");
    close(fd);
    return STATUS_ERROR;
  }
  if (listen(fd, 0) == -1) {
    perror("listen");
    close(fd);
    return STATUS_ERROR;
  }
  printf("Listening on port %d\n", port);
  return fd;
}
