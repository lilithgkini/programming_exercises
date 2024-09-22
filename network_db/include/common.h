#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#define STATUS_ERROR   -1
#define STATUS_SUCCESS 0

#define PORT 4444
#define BUFF_SIZE 4096

#define PROTO_VERSION 1

typedef enum{
	MSG_HELLO_REQ,
	MSG_HELLO_RES,
	MSG_EMPLOYEE_LIST_RES,
	MSG_EMPLOYEE_LIST_REQ,
	MSG_EMPLOYEE_ADD_RES,
	MSG_EMPLOYEE_ADD_REQ,
	MSG_ERROR,
} proto_state_e;

typedef struct{
	proto_state_e state;
	uint16_t len;
} proto_hdr_t;

typedef struct{
	uint16_t version;
} proto_hello_req;

typedef struct{
	proto_state_e state;
} proto_hello_res;

typedef struct{
	char buf[BUFF_SIZE];
} proto_employee_add_req;

typedef struct{
	char name[256];
	char address[256];
	unsigned int hours;
} proto_employee_list_res;

#endif
