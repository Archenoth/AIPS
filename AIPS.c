/* Archenoth IPS AIPS (Pronounced "Apes") */

#include "AIPS.h"
#include "IPS.h"
#include "UPS.h"

int main(int argc, char *argv[]) {
  pStruct params = {NULL, 0, NULL, NULL};

  int i;
  for(i = 1; i < argc; i++) {
    if(!parseArg(argv[i], &params)) {
      return 1;
    }
  }

  /* Help screen */
  if(params.flags & ARG_HELP) {
    printf("Archenoth IPS help.\n\n"
           "Invocation: %s <options> <IPS FIle> <ROM File>\n\n"
           "Options:\n"
           "-h, -help, -?, --help\tShows this help screen.\n"
           "-version\t\tPrints out version information.\n"
           "-v, -verbose\t\tShow verbose output. (Can be used twice.)\n",
           argv[0]);

    return 0;
  }

  /*
   * Version output, incomplete file error, and patch
   * operations. (Mutually exclusive)
   */
  if(params.flags & ARG_VERSION) {
    printf("Archenoth IPS version %s\n", VERSION);
  } else if(params.romFile == NULL || params.patchFile == NULL) {
    fprintf(stderr, "File to patch and patch file are both required.\n"
            "Try %s -h\n", argv[0]);
  } else {
    return params.patchFunction(&params);
  }

  return 1;
}

/**
 * Command line argument parsing function.
 *
 * This function parses arguments passed to AIPS from the command line
 * and sets verious flags according to their values.
 *
 * @param char *argument the argument string. (Taken directly from the
 * command line)
 * @param struct pStruct *params The parameter struct which holds
 * filen streams as well as all of the flags set combined with bitwise
 * math.
 *
 * @return 1 on success.
 */
int parseArg(char *argument, pStruct *params){
  if(argument[0] == '-') {
    if(strcmp(argument, "--version") == 0){
      /* Version */
      params->flags = params->flags | ARG_VERSION;
    } else if(strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0) {
      /* The help flag */
      params->flags |= ARG_HELP;
    } else if(strcmp(argument, "--verbose") == 0 || strcmp(argument, "-v") == 0){
      /* Verbosity flags */
      if(params->flags & ARG_VERBOSE) {
        params->flags |= ARG_VERYVERBOSE;
      } else {
        params->flags |= ARG_VERBOSE;
      }
    } else {
      return AIPSError(ERR_MEDIUM, "Unrecognized argument: %s\n", argument);
    }
  } else {
    return fileArgument(argument, params);
  }
  return 1;
}

/**
 * Filename parsing function
 *
 * This function parses filename arguments and attempts to open the
 * files associated with them. It also has the first level of error
 * handling for files.
 *
 * @param char *argument A pointer to an filename string to attempt to
 * open.
 * @param struct pStruct *params A pointer to a parameter struct
 * containing files, and flags for execution.
 *
 * @return Returns 1 on success, or 0 on failure.
 */
int fileArgument(char *argument, pStruct *params) {
  FILE *file;

  if((file = openIfPatch(argument, params))){
    if(params->patchFile == NULL) {
      /* TODO check size of file, if 0 set as create patch file */
      return !!(params->patchFile = file);
    } else {
      return AIPSError(ERR_MEDIUM, "You totally just gave me two patch files.");
    }
  } else {
    if(params->romFile == NULL) {
      return !!(params->romFile = useFile(argument, params, "rb+"));
    } else {
      return AIPSError(ERR_MEDIUM, "Hunh? Dual ROM files?");
    }
  }
}

/**
 * Using the enabled functions, this function will probe the passed-in
 * file handle, and see if it can figure out what kind of patch it's
 * looking at, and set the correct function in the passed in pStruct
 * accordingly.
 *
 * @param FILE* file: A pointer to the file you wish to probe.
 * @param pStruct* params: A pointer to the pStruct you wish to set
 * the correct patching function to patch the returned file with.
 *
 * @return FILE*: A patchable file if we found a patching function, or
 * NULL if nothing is found.
 */
FILE* headerProbe(FILE* file, pStruct *params){
  int (*function[])(FILE *argFile, int verbose) = {
    IPSCheckPatch,
    UPSCheckPatch
  };

  int (*patchFunction[])(pStruct *params) = {
    IPSPatchFile,
    UPSPatchFile
  };

  int i;
  for(i = 0; i < (int)(sizeof(function)/sizeof(function[0])); i++) {
    if(function[i](file, (params->flags & ARG_VERBOSE))){
      params->patchFunction = patchFunction[i];
      return file;
    } else {
      rewind(file); /* So that the next check will happen from the beginning. */
    }
  }

  /* This file is obviously not a valid patch at this point in time... */
  fclose(file);
  if(params->flags & ARG_VERBOSE) {
    printf("This looks like the ROM file to patch..?\n");
  }

  return NULL;
}

/**
 * Determines if the file passed in is a patch or not.
 *
 * This function will take a char[] string filename, open it, then try
 * its best to figure out what kind of file it is. It will print its
 * findings out to the console
 *
 * If the file passed in doesn't exist, it is created and returned as
 * a pointer to the writable FILE for use with fwrite and other
 * writing stream functions.
 *
 * @param char* argument: The filename passed into the function.
 * @param int verbose: Will determine if the function prints out
 * information to the console about its findings. It will print them
 * if true, or else it will be silent.
 *
 * @return int: Returns the FILE if the passed in argument is a path
 * to a valid patch file, and NULL otherwise.
 */
FILE* openIfPatch(char *filename, pStruct *params) {
  FILE *file;

  if(!(file = useFile(filename, params, "rb"))){
    return useFile(filename, params, "wb+");
  }

  /*
   * Least amount of work... Just check the filename to determine type
   * of check.
   */
  if(strlen(filename) > 4) {
    char *extension = filename + (strlen(filename) - 4);

    /* Quick IPS check */
    if(strcasecmp(extension, ".ips") == 0) {
      if(IPSCheckPatch(file, (params->flags & ARG_VERBOSE))) {
        params->patchFunction = &IPSPatchFile;
        return file;
      }
    }

    /* Quick UPS check */
    if(strcasecmp(extension, ".ups") == 0) {
      if(UPSCheckPatch(file, (params->flags & ARG_VERBOSE))) {
        params->patchFunction = &UPSPatchFile;
        return file;
      }
    }
  }

  /* Hoomy. Guess we gotta check the headers of the files themselves. */
  return headerProbe(file, params);
}



/**
 * Opens a file with a debug and error handling wrapper.
 *
 * This function is a very loose abstraction of the fopen function
 * that will accept a path to a file, and a pStruct that contains the
 * argument flags that you had set before calling this function, a
 * file pointer that you wish to store the stream, and finally the
 * mode that you wish to use to open the file with.
 *
 * @param char *argument A string of the path to the file you are
 * looking to open.
 * @param struct pStruct *params A struct containing a valid flags
 * variable to determine the options for opening the file and function
 * operation.
 * @param FILE *argFile a file pointer that you wish to open the file
 * stream to.
 * @param char *mode A string denoting the mode that you wish to open
 * the file with.
 *
 * @return Returns a pointer to the FILE when it succeeds, or NULL
 * when it does not.
 */
FILE *useFile(char *argument, pStruct *params, char *mode) {
  FILE *argFile = fopen(argument, mode);

  if((params->flags & ARG_VERBOSE)){
    if(argFile == NULL) {
      printf("Creating patch file: %s\n", argument);
    } else {
      printf("Using file: %s\n", argument);
    }
  }

  return argFile;
}


/**
 * Error printing function.
 *
 * This function takes an error level and a string or two, then
 * concatenates the strings, prints them to stderr, and takes the
 * requested action according to the error level.
 *
 * @param int level The level of the error passed ERR_MINOR is more
 * like a warning, and returns a 1, allowing operations to
 * continue. ERR_MEDIUM is a standard error that will returna 0 status
 * so correct handling can take place, and ERR_MAJOR halts the
 * program.
 * @param char *message A message that you wish to print to the
 * screen, accepts printf-style escapes and variables.
 * @param ... The remainder of the arguments that will be put into the
 * string in lieu of printf.
 *
 * @return int: Returns the error status that can likely kjust be
 * returned from the function that called this function.
 */
int AIPSError(int level, const char *message, ...){
  va_list concat;

  if(level > ERR_MINOR) {
    fprintf(stderr, "Error: ");
  }

  va_start(concat, message);
  vfprintf(stderr, message, concat);
  va_end(concat);
  fprintf(stderr, "\n");

  switch(level) {
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
