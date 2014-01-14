FILE* openFile(char *fileName, FILE **fp);
struct request* createFileArray(int filesize);
void closeFile(FILE *fp);
int saveFile(FILE *fp, struct request* FileArray,int sizeFA);