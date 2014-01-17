FILE* openFile(char *fileName);
int getFileSize();
//char* getChars(FILE *fp,int anzZeichen, int pos);
struct fileread* getChars(FILE *fp,int anzZeichen, int pos);
struct request* createFileArray(char *fileName,int *filearrysize);
void closeFile(FILE *fp);
