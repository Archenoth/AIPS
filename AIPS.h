#ifndef AIPS_HEAD
#define AIPS_HEAD

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define VERSION "0.0.1"

// Argument definition
#define ARG_VERSION (1 << 0)
#define ARG_HELP (1 << 1)
#define ARG_VERBOSE (1 << 2)
#define ARG_OVERWRITE (1 << 3)

// Error level definition
#define ERR_MINOR 0
#define ERR_MEDIUM 1
#define ERR_MAJOR 2

// Conversion macros courtesy of ZeroSoft on Zophar's domain
// http://zerosoft.zophar.net/ips.php
#define BYTE3_TO_UINT(bp) \
  (((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
  (((unsigned int)(bp)[1] << 8) & 0x0000FF00) |	 \
  ((unsigned int)(bp)[2] & 0x000000FF)

#define BYTE2_TO_UINT(bp)		    \
  (((unsigned int)(bp)[0] << 8) & 0xFF00) | \
  ((unsigned int) (bp)[1] & 0x00FF)

// Parameter Struct
struct pStruct
{
  int flags;
  FILE *romFile;
  FILE *patchFile;
};

// Function definitions
int parseArg(char *argument, struct pStruct *params);
int fileArgument(char *argument, struct pStruct *params);
int AIPSError(int level, const char *message, ...);
int patchROM(struct pStruct *params);
int useFile(char *argument, struct pStruct *params, FILE *argFile, char *mode);
int isPatch(char *argument, int verbose);
#endif
