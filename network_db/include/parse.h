#ifndef PARSE_H
#define PARSE_H
#include <stdbool.h>

#define MAGIC_VALUE 0x43410703

struct dbheader_t {
	unsigned int magic;
	unsigned short version;
	unsigned short count;
	unsigned int filesize;
};

struct employee_t {
	char name[256];
	char address[256];
	unsigned int hours;
};

int get_header(int, bool, struct dbheader_t**);
int read_employees(int, struct dbheader_t*, struct employee_t**);
int add_employee(struct dbheader_t*, struct employee_t**, char*);
void list_employees(struct dbheader_t*, struct employee_t*);

#endif
