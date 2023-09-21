/* #include <bits/getopt_core.h> */
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "mondrian.h"

static int anonymize_flag;
static int relaxed_flag;
static int results_flag;

int main(int argc, char **argv) {
  int opt;
  int opt_idx = 0;
  static struct option long_options[] = {
      /* flags */
      {"anonymize", no_argument, &anonymize_flag, 1},
      {"relaxed", no_argument, &relaxed_flag, 1},
      {"results", no_argument, &results_flag, 1},
      /* Mondrian options */
      {"input", required_argument, 0, 'i'},
      {"k", required_argument, 0, 'k'},
      {"output", required_argument, 0, 'o'},
      /* extra */
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  int digit = 0;
  void *fp;
  while ((opt = getopt_long(argc, argv, "hi:k:o:", long_options, &opt_idx)) !=
         -1) {
    switch (opt) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[opt_idx].flag != 0)
        break;
    case 'i':
      DATASET = optarg;
      break;
    case 'o':
      OUTPUT = optarg;
      break;
    case 'k':
      for (int i = 0; i < strlen(optarg); i++) {
        if (!isdigit(optarg[i])) {
          fprintf(stderr, "k-Anonymity value must be integer\n");
          exit(1);
        }
      }
      GL_K = atoi(optarg);
      break;
    case 'h':
      usage(0);
      break;
    case '?':
      /* getopt_long already printed an error message. */
      break;
    default:
      usage(1);
    }
  }

  if (!DATASET)
    DATASET = "../datasets/adults.csv";
  if (!OUTPUT)
    OUTPUT = "output.csv";
  if (relaxed_flag)
    MODE = "relaxed";
  else
    MODE = "strict";
  if (results_flag)
    RES = 1;
  if (anonymize_flag)
    ANON = 1;
  parse_dataset();
  mondrian();
  free_mem();
  return 0;
}
