// UPS file-specific functions

#include "AIPS.h"
#include "UPS.h"
#include "CRC.h"

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
  if(fread(buffer, BYTE, 4, filePointer) == 0)
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
 * @param FILE *filePointer the pointer to the UPS patch file.
 * @returns int 0 on a failed read.
 */
int UPSReadRecord(FILE *filePointer)
{

  /* unsigned int offset[7], bit[1]; */
  /* fread(&offset, 1, 7, filePointer); */
  /* fread(&bit, 1, 1, filePointer); */
  return 0;
}

/*
 * Reads a single variable-length encoded integer at the current point
 * of the file pointer passed in, and sets the file to the end of the
 * VLE.
 * @param FILE *file the pointer to the file to read the VLE int from.
 * @returns int the integer from the file.
 */
int readVLE(FILE* file)
{
  int shift = 1,
      buffer = 0,
      result = 0;
  
  while(1){
    fread(&buffer, BYTE, 1, file);
    result += (buffer & 0x7f) * shift;

    // If the encoding's high bit is set, we are done here.
    if(buffer & 0x80)
      break;

    // Add a new octet sans one bit (The "Should we continue?" bit.)
    result += (shift <<= 7);
  }
  return result;
} 

/*
 * Patches a UPS file according to the paramaters passed in the
 * pStruct.
 * @param pStruct *params A paramater structure complete with the
 * input and output files, as well as any flags wanted during
 * patching.
 * @returns int 1 on a sucessful patch, 0 otherwise.
 */
int UPSPatchFile(struct pStruct *params)
{
  fseek(params->romFile, 0, SEEK_END);

  int inputFileSize = readVLE(params->patchFile),
      outputFileSize = readVLE(params->patchFile),
      actualSize = ftell(params->romFile);
  
  rewind(params->romFile);
  if(params->flags & ARG_VERBOSE)
    printf("The UPS patch says:\n"
	   "Input Filesize: %d bytes\nOutput Filesize: %d bytes\n"
	   "...And the actual filesize is: %d bytes\n",
	   inputFileSize, outputFileSize, actualSize);

  if(inputFileSize != actualSize) //TODO: Force option.
    return AIPSError(ERR_MEDIUM, "The file seems to be the wrong size, yo~!");

  if(params->flags & ARG_VERBOSE)
    printf("Good! They match. Now for the CRC checks..!\n");


  long position = ftell(params->patchFile);

  fseek(params->patchFile, -12L, SEEK_END);
  unsigned int inputChecksum,
               outputChecksum,
               patchChecksum;
  if(fread(&inputChecksum, BYTE, 4, params->patchFile) &&
     fread(&outputChecksum, BYTE, 4, params->patchFile) &&
     fread(&patchChecksum, BYTE, 4, params->patchFile))
    {
      fseek(params->patchFile, -4L, SEEK_END);
      unsigned int table[256];
      
      //UPS uses 0xedb88320 as the polynomial value for CRC.
      crcTable(&table[0], 0xedb88320);

      unsigned int patchFileCRC = crcFile(params->patchFile, &table[0]),
	           romFileCRC = crcFile(params->romFile, &table[0]);

      if(params->flags & ARG_VERBOSE)
	printf("Patch file read CRC: %#10x, actual: %#10x\n"
	       "ROM file read CRC: %#10x, actual: %#10x\n",
	       patchChecksum, patchFileCRC,
	       inputChecksum, romFileCRC);

      if(patchChecksum != patchFileCRC)
	return AIPSError(ERR_MEDIUM,
			 "Oh no! This patch file looks invalid!");
      
      if(inputChecksum != romFileCRC)
	return AIPSError(ERR_MEDIUM,
			 "You may have an invalid file."
			 " (Or this patch isn't for this file.)");
    }
  
  fseek(params->patchFile, position, SEEK_SET);
  //TODO: The actual patch
  
  return 0;
}
