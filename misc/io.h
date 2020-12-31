#ifndef IO_H
#define IO_H

typedef struct flist {
	char* name;
	char* fullname;
	char* outname;
	struct flist* next;
} FILELIST;


FILELIST* getfiles(char* input);
void freefilelist(FILELIST* fs);
#endif
