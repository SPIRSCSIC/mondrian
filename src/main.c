#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "mondrian.h"

int main(int argc, char **argv) {
  int opt;
  int errflg = 0;
  int digit = 0;
  void *fp;
  while ((opt = getopt(argc, argv, ":f:m:o:hak:r")) != -1) {
    switch (opt) {
    case 'f':
      if (access(optarg, F_OK)) {
        fprintf(stderr, "Error: File %s does not exist or cannot be opened\n",
                optarg);
        exit(errno);
      }
      DATASET = optarg;
      break;
    case 'o':
      fp = fopen(optarg, "w");
      if (fp != NULL) {
        fclose(fp);
        remove(optarg);
      } else {
        fprintf(stderr, "Error: File %s cannot not be written\n", optarg);
        exit(errno);
      }
      OUTPUT = optarg;
      break;
    case 'm':
      if (strcmp(optarg, "strict") && strcmp(optarg, "relaxed")) {
        fprintf(stderr,
                "Error: Option -m requires argument: strict or relaxed\n");
        errflg++;
        break;
      }
      MODE = optarg;
      break;
    case 'k':
      for (int i = 0; optarg[i]; i++) {
        if ((optarg[i] != '\0') && (digit = isdigit(optarg[i])))
          continue;
        else {
          fprintf(stderr, "Error: Option -k requires an integer argument\n");
          errflg++;
          break;
        }
      }
      GL_K = atoi(optarg);
      break;
    case 'a':
      ANON = 1;
      break;
    case 'r':
      RES = 1;
      break;
    case ':':
      /* -f or -o without operand */
      fprintf(stderr, "Error: Option -%c requires an argument\n", optopt);
      errflg++;
      break;
    case 'h':
      usage(0);
    case '?':
      fprintf(stderr, "Error: Unrecognized option: '-%c'\n", optopt);
      errflg++;
    }
  }
  if (errflg) {
    usage(1);
  }
  if (!DATASET)
    DATASET = "../datasets/adults.csv";
  if (!OUTPUT)
    OUTPUT = "output.csv";
  if (!MODE)
    MODE = "strict";
  parse_dataset();
  mondrian();
  free_mem();
  return 0;
}
