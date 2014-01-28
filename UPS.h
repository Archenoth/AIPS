int UPSReadRecord(FILE *filePointer);
int UPSCheckPatch(FILE *filePointer, int verbose);
int UPSPatchFile(struct pStruct *params);
int readVLE(FILE* file);
