// Version number
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

// Conversion macros
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
  FILE *ipsFile;
};

struct patchData
{
  int offset;
  int size;
  char *data;
};

// Function definitions
int parseArg(char *argument, struct pStruct *params);
int fileArgument(char *argument, struct pStruct *params);
int AIPSError(int level, const char *message, ...);
int patchROM(struct pStruct *params);
int readRecord(struct patchData *patch, FILE *filePointer);
int readRLE(struct patchData *patch, FILE *filePointer);
