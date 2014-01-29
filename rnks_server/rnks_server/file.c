#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include "data.h"

struct stat *fileInfo;
int filesize;

FILE* openFile(char *fileName){
	FILE *fp;
	int i=0;
	fp = fopen(fileName,"r"); //datei zum lesen öffnen
	if(fp == NULL) return 0; //datei existiert nicht oder konnte nicht geöffnet werden
	fileInfo = (struct stat *)malloc(sizeof(struct stat));
	//schreibe anzahl zeichen der datei
	stat(fileName,fileInfo);//hole info zur datei
	fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	return fp;
}

int getFileSize(){
	return fileInfo->st_size;
}

struct fileread* getChars(FILE *fp,int anzZeichen, int pos){
	
	int anzLsbZeichen=0;
	//char buff[PufferSize];
	struct fileread buff; //evtl als pointer erstellen mit malloc besser ??

	if(fp != NULL){ //ist dateizeiger geöffnet?
		if(pos > filesize) return NULL;//existiert die gesuchte pos?
		if((anzLsbZeichen = (filesize-(pos))) < anzZeichen ) 
			anzZeichen = anzLsbZeichen;//wie viele zeichen können überhaupt noch gelesen werden?
			
		if(anzZeichen != PufferSize) buff.ccount=fread(buff.rbuff,sizeof(char),(anzZeichen-1)*sizeof(char),fp);//lese
		else buff.ccount=fread(buff.rbuff,sizeof(char),(anzZeichen)*sizeof(char),fp);//lese
		return &buff;//gibt die fileread struktur zurück
		
	}
	return NULL;
}

void closeFile(FILE *fp){
	free(fileInfo);
	if(fp != NULL) fclose(fp);
}

struct request* createFileArray(char *fileName,int *filearrysize){
	int i=0,buf,tmpSize; //faktor für pos berechnung
	struct request *FileArray;
	struct fileread *tmp=NULL; 
	FILE *fp=NULL;
	
	
	buf = (int)PufferSize;
	if(fp = openFile(fileName)){

		FileArray = (struct request *) malloc(((int)(filesize/buf)+1)*sizeof(struct request));
		
		while(i < ((int)(filesize/buf)+1)){
			tmp = getChars(fp,buf, i*buf);
			
			//info in req 
			tmpSize = buf;
			if((int)(filesize/buf) == i) tmpSize = filesize%buf;
			memcpy(FileArray[i].name,tmp->rbuff,tmp->ccount*sizeof(char)); //daten
			FileArray[i].FlNr = tmp->ccount; //datenlänge
			FileArray[i].ReqType = ReqData; //reqtype
			FileArray[i].SeNr = i;//sqnr
			memcpy(FileArray[i].fname,fileName,strlen(fileName)+1);
			FileArray[i].fname[strlen(fileName)] = '\0';
			i++;
		}
		
		closeFile(fp);
		
		*filearrysize=i;//hält global die array größe [user ist die funktion arraychaos(...)]
		return FileArray;
	}
}

