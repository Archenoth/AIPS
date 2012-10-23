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

  if(params.flags & ARG_VERSION)
    printf("Archenoth IPS version %s\n", VERSION);
  else if(params.romFile == NULL ||
	  params.ipsFile == NULL)
    fprintf(stderr, "File to patch and patch file are both required.\n");
  else
    return patchROM(&params);
    
  return 0;
}


int parseArg(char *argument, struct pStruct *params)
{
if(argument[0] == '-')
  {
    if(strcmp(argument, "-version") == 0)
      params->flags = params->flags + ARG_VERSION;
    if(strcmp(argument, "-verbose") == 0 ||
       strcmp(argument, "-v") == 0)
      params->flags = params->flags + ARG_VERBOSE;
  }
 else
   return fileArgument(argument, params);
 return 1;
}


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
	  params->romFile = fopen(argument, "rb");
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


int AIPSError(int level, const char *message, ...)
{
  va_list concat;

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
      printf("HAHAHA%s\n\n\n\n\nNONONO\n", (char*)patch.data);
      free(patch.data);
      //      printf("%zd", patch.data);
      //free(patch.data);
      //      free(patch.data);
      //    fwrite(&patch.data, 1, sizeof(patch.data), stdout);
    }
  //printf("%s \n", patch.data);
  //fwrite(&buffer, sizeof(buffer), 1, stdout);
  return 0;
}

int readRecord(struct patchData *patch, FILE *filePointer)
{
  if(fread(&patch->offset, 1, 3, filePointer) &&
     fread(&patch->size, 1, 2, filePointer))
    {
      if(patch->size == 0)
	return readRLE(patch, filePointer);
      //     char *data[(int)(sizeof(char) / patch->size) + 1];
  //      char *data = malloc(sizeof(char) * patch->size);
      //      patch->data = ()realloc(patch->data, patch->size + 1);
      patch->data = (char*)malloc(patch->size + 1);
      //      int i = 0;

      //      printf("%d", (int)sizeof(patch->data));
      //      while(patch->size > sizeof(patch->data))
      return fread(patch->data, patch->size, 1, filePointer);
      /* 	  return 0; */
      /* return 1; */
    }
  return 0;
}


int readRLE(struct patchData *patch, FILE *filePointer)
{
  if(fread(&patch->size, 16, 1, filePointer))
    {
      printf("%d", patch->size);
      /* int *byte = NULL, i; */
      /* if(fread(byte, 8, 1, filePointer)) */
      /* 	for(i = 0; i > patch->size; i++) */
      /* 	  printf("%d", *byte); */
    }
  return 0;
}
