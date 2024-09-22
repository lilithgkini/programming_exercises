#include "../../include/common.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void get_help(char **argv) {
  printf("Usage: %s -f <file> [options] \n", argv[0]);
  printf("\n");
  printf("Options:\n");
  printf("\t-h - This help\n");
  printf("\t-f - Database name to open\n");
  printf("\t-n - Create a new database\n");
  exit(-1);
  return;
}

int parse_args(int argc, char **argv, bool *newfile, char **filepath) {
  int c;
  while ((c = getopt(argc, argv, "hnf:")) != -1) {
    switch (c) {
    case 'n':
      *newfile = true;
      break;
    case 'f':
      *filepath = optarg;
      break;
    case 'h':
      get_help(argv);
      break;
    case '?':
      printf("Unknown option -%c\n", c);
      get_help(argv);
      break;
    default:
      get_help(argv);
      return STATUS_ERROR;
    }
  }
  return STATUS_SUCCESS;
}
