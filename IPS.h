struct patchData
{
  unsigned int offset;
  unsigned int size;
  char *data;
};

int IPSReadRecord(struct patchData *patch, FILE *filePointer);
int IPSReadRLE(struct patchData *patch, FILE *filePointer);
int IPSCheckPatch(FILE *filePointer, int verbose);
