FILE* openFile(char *fileName);
int getFileSize();
char* getChars(FILE *fp,int anzZeichen, int pos);
struct request* createFileArray(char *fileName);
void closeFile(FILE *fp);