#ifndef FILE_H
#define FILE_H
#include <stdbool.h>
#include "parse.h"

int get_db_file(char *, bool *);
int write_db_file(int, struct dbheader_t*, struct employee_t*);

#endif
