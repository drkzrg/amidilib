#ifndef GETOPT_H_
#define GETOPT_H_

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN int getopt(int argc, char * const argv[], const char *optstring);

EXTERN char *optarg;
EXTERN int optind;
EXTERN int opterr;
EXTERN int optopt;

#endif
