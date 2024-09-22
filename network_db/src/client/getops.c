#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void get_help(char **argv) {
  printf("Usage: %s -f <file> [options] \n", argv[0]);
  printf("\n");
  printf("Options:\n");
  printf("\t-h - This help\n");
  printf("\t-a Add employees \"<name>,<address>,<hours>\"\n");
  printf("\t-l List employees\n");
  printf("\n");
  printf("Example: %s -a \"Foo,Bar str,120\" -l\n", argv[0]);

  exit(-1);
  return;
}

int parse_args(int argc, char **argv, bool *list, char **add_emp) {
  int c;
  while ((c = getopt(argc, argv, "hla:")) != -1) {
    switch (c) {
    case 'l':
      *list = true;
      break;
    case 'a':
      *add_emp = optarg;
      break;
    case 'h':
      get_help(argv);
      break;
    case '?':
      printf("Unknown option -%c\n", c);
      get_help(argv);
      break;
    default:
      return -1;
    }
  }
  return 0;
}
