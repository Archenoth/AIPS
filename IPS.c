// IPS specific functions

#include "AIPS.h"
#include "IPS.h"

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
int IPSReadRecord(struct patchData *patch, FILE *filePointer)
{
  unsigned char offset[3], size[2];
  if(fread(&offset, 1, 3, filePointer) &&
     fread(&size, 1, 2, filePointer))
    {      
      // Fix linear reads
      patch->size = BYTE2_TO_UINT(size);
      patch->offset = BYTE3_TO_UINT(offset);

      if(patch->size == 0)
	return IPSReadRLE(patch, filePointer);

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
int IPSReadRLE(struct patchData *patch, FILE *filePointer)
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


/*
 * Checks that an IPS file has the correct header
 *
 * Checks that the next 5 bytes (In an IPS file, the first 5 bytes are
 * where the header is present) say "PATCH".
 * @param FILE *filePointer the pointer to the IPS patch file.
 * @returns int 1 on a valid PATCH header, 0 otherwise.
 */
int IPSCheckPatch(FILE *filePointer, int verbose)
{
  char buffer[6];
  if(fread(buffer, 1, 5, filePointer) == 0)
    return 0;

  buffer[5] = '\0';

  // Valid patch header?
  if(strcmp(buffer, "PATCH") == 0)
    {
      if(verbose)
	printf("This appears to be a valid IPS patch file...\n");
      return 1;
    }
  return 0;
}

/*
 * Patches a file using an IPS file
 *
 * This function will attempt to patch a file specified in it's
 * parameters with the patch file also specified in it's parameters
 * according to the flags set.
 * @param struct pStruct *params A pointer to a parameter struct that
 * contains the files and parameters in which to patch the file.
 * @return Returns 1 on success or 0 on failure.
 */
int IPSPatchFile(struct pStruct *params)
{
  struct patchData patch = {};
  while(IPSReadRecord(&patch, params->patchFile))
    {
      if((params->flags & ARG_VERYVERBOSE))
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
 * Creates a patch file with the passed in pStruct
 *
 * This function creates an IPS patch file
 */
int IPSCreatePatch(struct pStruct *params)
{
  rewind(params->patchFile);
  fwrite("PATCH", 1, sizeof("PATCH"), params->patchFile);
  
  fwrite("EOF", 1, sizeof("EOF"), params->patchFile);
  return 0;
}


int IPSWriteRecord(struct patchData *patch, FILE *filePointer)
{
  fwrite(&patch->offset, 3, 1, filePointer);
  fwrite(&patch->size, 2, 1, filePointer);
  fwrite(&patch->data, sizeof(patch->data), 1, filePointer);
  return 0;
}


int IPSWriteRLE(struct patchData *patch, FILE *filePointer)
{
  fwrite(&patch->offset, 2, 1, filePointer); //Offset should be 0
  fwrite(&patch->size, 2, 1, filePointer);
  fwrite(&patch->data, 1, 1, filePointer);
  return 0;
}
