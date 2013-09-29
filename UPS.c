// UPS file-specific functions

#include "AIPS.h"
#include "UPS.h"

/*
 * Checks that a UPS file has the correct header
 *
 * Checks that the next 4 bytes (In an UPS file, the first 4 bytes are
 * where the header is present) say "UPS1".
 * @param FILE *filePointer the pointer to the UPS patch file.
 * @returns int 1 on a valid UPS1 header, 0 otherwise.
 */
int UPSCheckPatch(FILE *filePointer, int verbose)
{
  fseek(filePointer, 0L, SEEK_END);
  long fileSize = ftell(filePointer);
  rewind(filePointer);

  if(fileSize < 20) // Not long enough to be a UPS patch file
    return 0;

  char buffer[5];
  if(fread(buffer, 1, 4, filePointer) == 0)
    return 0;

  buffer[4] = '\0';

  // Valid patch header?
  if(strcmp(buffer, "UPS1") == 0)
    {
      if(verbose)
	printf("This appears to be a valid UPS patch...\n");
      return 1;
    }
  return 0;
}

/*
 * Reads a single Record from the patch file and returns it in a
 * UPS patchData struct.
 */
int UPSReadRecord(FILE *filePointer)
{
  fseek(filePointer, -12L, SEEK_END);
  char *inputChecksum = NULL,
       *outputChecksum = NULL,
       *patchChecksum = NULL;
  if(fread(inputChecksum, 1, 4, filePointer) &&
     fread(outputChecksum, 1, 4, filePointer) &&
     fread(patchChecksum, 1, 4, filePointer))
    {}

  /* unsigned int offset[7], bit[1]; */
  /* fread(&offset, 1, 7, filePointer); */
  /* fread(&bit, 1, 1, filePointer); */
  return 0;
}

int UPSPatchFile(struct pStruct *params)
{
  return 0;
}
