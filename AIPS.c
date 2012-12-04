// Archenoth IPS AIPS (Pronounced "Apes")

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "AIPS.h"


int main(int argc, char *argv[])
{
  struct pStruct params = {.romFile = NULL, .ipsFile = NULL};

  int i;
  for(i = 1; i < argc; i++)
    if(!parseArg(argv[i], &params))
      return 1;

  if(params.flags & ARG_HELP)
    return AIPSError(ERR_MINOR,
		     "Archenoth IPS help.\n\n"
		     "Invocation: %s <IPS FIle> <ROM File> <options>\n\n"
		     "-h, -help, -?, --help\tShows this help screen.\n"
		     "-version\t\tPrints out version information.\n"
		     "-v, -verbose\t\tShow verbose output.\n", argv[0]);

  if(params.flags & ARG_VERSION)
    printf("Archenoth IPS version %s\n", VERSION);
  else if(params.romFile == NULL ||
	  params.ipsFile == NULL)
    fprintf(stderr, "File to patch and patch file are both required.\n"
	    "Try %s -h\n", argv[0]);
  else
    return patchROM(&params);
    
  return 0;
}

/*
 * Command line argument parsing function.
 *
 * This function parses arguments passed to AIPS from the command line
 * and sets verious flags according to their values.
 * @param char *argument the argument string. (Taken directly from the
 * command line)
 * @param struct pStruct *params The parameter struct which holds
 * filen streams as well as all of the flags set combined with bitwise
 * math.
 * @return 1 on success.
 */
int parseArg(char *argument, struct pStruct *params)
{
if(argument[0] == '-')
  {
    if(strcmp(argument, "-version") == 0)
      params->flags = params->flags + ARG_VERSION;
    if(strcmp(argument, "-verbose") == 0 ||
       strcmp(argument, "-v") == 0)
      params->flags = params->flags + ARG_VERBOSE;
    if(strcmp(argument, "-help") == 0 ||
       strcmp(argument, "-h") == 0 ||
       strcmp(argument, "--help") == 0 ||
       strcmp(argument, "-?") == 0)
      params->flags = params->flags + ARG_HELP;
  }
 else
   return fileArgument(argument, params);
 return 1;
}

/*
 * Filename parsing function
 *
 * This function parses filename arguments and attempts to open the
 * files associated with them. It also has the first level of error
 * handling for files.
 * @param char *argument A pointer to an filename string to attempt to
 * open.
 * @param struct pStruct *params A pointer to a parameter struct
 * containing files, and flags for execution.
 * @return Returns 1 on success, or 0 on failure.
 */
int fileArgument(char *argument, struct pStruct *params)
{
  if(params->ipsFile)
    {
      if(params->romFile)
	return AIPSError(ERR_MEDIUM,
			 "What? THREE files? You may want to try "
			 "that again...\n");
      else
	{
	  params->romFile = fopen(argument, "rb+");
	  if((params->flags & ARG_VERBOSE))
	    printf("ROM File to use is: %s\n", argument);
	  if(params->romFile == NULL)
	    return AIPSError(ERR_MEDIUM,
			     "Cannot open ROM file: %s", argument);
	}
    }
  else
    {
      params->ipsFile = fopen(argument, "r");
      if((params->flags & ARG_VERBOSE))
	printf("IPS file is: %s\n", argument);
      if(params->ipsFile == NULL)
	    return AIPSError(ERR_MEDIUM,
			     "Cannot open IPS file: %s", argument);
    }
  return 1;
}

/*
 * Error function.
 *
 * This function takes an error level and a string or two, then
 * concatenates the strings, prints them to stderr, and takes the
 * requested action according to the error level.
 * @param int level The level of the error passed ERR_MINOR is more
 * like a warning, and returns a 1, allowing operations to
 * continue. ERR_MEDIUM is a standard error that will returna 0 status
 * so correct handling can take place, and ERR_MAJOR halts the
 * program.
 * @param char *message A message that you wish to print to the
 * screen, accepts printf-style escapes and variables.
 * @param ... The remainder of the arguments that will be put into the
 * string in lieu of printf.
 */
int AIPSError(int level, const char *message, ...)
{
  va_list concat;

  if(level > ERR_MINOR)
    fprintf(stderr, "Error: ");

  va_start(concat, message);
  vfprintf(stderr, message, concat);
  va_end(concat);
  fprintf(stderr, "\n");

  switch(level)
    {
    case ERR_MINOR:
	return 1;
    case ERR_MEDIUM:
	return 0;
    case ERR_MAJOR:
	_Exit(1);
    default:
      return AIPSError(ERR_MEDIUM, "Unrecognized error!");
    }
}

/*
 * Patches a file
 *
 * This function will attempt to patch a file specified in it's
 * parameters with the patch file also specified in it's parameters
 * according to the flags set.
 * @param struct pStruct *params A pointer to a parameter struct that
 * contains the files and parameters in which to patch the file.
 * @return Returns 1 on success or 0 on failure.
 */
int patchROM(struct pStruct *params)
{
  char buffer[6];
  if(fread(buffer, 1, 5, params->ipsFile) == 0)
    return 1;

  buffer[5] = '\0';

  // Valid patch header?
  if(strcmp(buffer, "PATCH") != 0)
    AIPSError(ERR_MAJOR, "Invalid patch file! %s", buffer);

  struct patchData patch = {};
  while(readRecord(&patch, params->ipsFile))
    {
      if((params->flags & ARG_VERBOSE))
	printf("Applied patch. Offset: Byte %d size: %d bytes\n",
	       (unsigned int)patch.offset,
	       (unsigned int)patch.size);

      fseek(params->romFile, patch.offset, SEEK_SET);
      fwrite(patch.data, patch.size, 1, params->romFile);
      free(patch.data);
    }
  fclose(params->romFile);
  return 0;
}

/*
 * Reads a record from the patch file.
 *
 * This code both detects, and reads the next record in the
 * filePointer at the given location and writes the results into the
 * patchData struct supplied.
 * @param struct *patchData A pointer to a patchData struct that the
 * information will be written to.
 * @param FILE *filePointer a pointer to the patch file being read.
 */
int readRecord(struct patchData *patch, FILE *filePointer)
{
  unsigned char offset[3], size[2];
  if(fread(&offset, 1, 3, filePointer) &&
     fread(&size, 1, 2, filePointer))
    {      
      // Fix linear reads
      patch->size = BYTE2_TO_UINT(size);
      patch->offset = BYTE3_TO_UINT(offset);

      if(patch->size == 0)
	return readRLE(patch, filePointer);

      patch->data = (char*)malloc(patch->size + 1);
      return fread(patch->data, patch->size, 1, filePointer);
    }
  return 0;
}

/*
 * Read an RLE-Encoded patch...
 * 
 * RLE patches are simple patches with a single byte repeating a
 * specified quantity of times. The first 16 bits are the size of the
 * patch, or the number of times to repeat the insert, and the value
 * immediately succeeding this number is the patch character itself...
 * @param struct patchData *patch A patch struct pointer holding the
 * offset, the size, and the data...
 * @param FILE *filePointer a file pointer to the current location in
 * the patch file.
 * @return Returns 1 on success, 0 on failure.
 */
int readRLE(struct patchData *patch, FILE *filePointer)
{
  char size[2];
  if(fread(&size, 2, 1, filePointer))
    {
      int count;
      char data = '\0';

      patch->size = BYTE2_TO_UINT(size);

      if(!fread(&data, 1, 1, filePointer))
	return 0;

      patch->data = (char*)malloc((patch->size * sizeof(char)) + 1);
      for(count = 0; count < patch->size; count++)
	patch->data[count] = (char)data;
      return 1;
    }
  return 0;
}
