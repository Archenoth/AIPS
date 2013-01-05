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
int UPSCheckPatch(FILE *filePointer)
{
  char buffer[5];
  if(fread(buffer, 1, 4, filePointer) == 0)
    return 1;

  buffer[4] = '\0';

  // Valid patch header?
  return strcmp(buffer, "UPS1") != 0;
}
