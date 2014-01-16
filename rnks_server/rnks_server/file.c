#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include "data.h"
/*
	- öffnen der file im textmodus
	- zerteilen der file in ein array a PufferSize-1 + 50-1 zeichen
	- schließen der file
*/

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

char* getChars(FILE *fp,int anzZeichen, int pos){
	int anzLsbZeichen=0;
	char buff[PufferSize];
	//anzZeichen--;//um 1 decrementieren um \0 anzufügen
	if(fp != NULL){ //ist dateizeiger geöffnet?
		//if(pos > fileInfo->st_size) return NULL;//existiert die gesuchte pos?
		if(pos > filesize) return NULL;//existiert die gesuchte pos?
		//if((anzLsbZeichen = (fileInfo->st_size-(pos))) < anzZeichen ) 
		if((anzLsbZeichen = (filesize-(pos))) < anzZeichen ) 
			anzZeichen = anzLsbZeichen;//wie viele zeichen können überhaupt noch gelesen werden?
			
		//buff = (char*) malloc((anzZeichen)*sizeof(char));//puffer anpassen
		//lese die zeichen...
		//fseek(fp,pos,SEEK_SET);//gehe an die richtige pos
		if(anzZeichen != PufferSize) fread(buff,sizeof(char),(anzZeichen-1)*sizeof(char),fp);//lese
		else fread(&buff,sizeof(char),(anzZeichen)*sizeof(char),fp);//lese
		//buff[anzZeichen] = '\0'; //stringende anfügen
		return buff;//gib den char* zurück
		
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
	char *tmp;
	FILE *fp=NULL;
	
	
	buf = (int)PufferSize;
	if(fp = openFile(fileName)){

		//FileArray = (struct request *) malloc(((int)(fileInfo->st_size/buf)+1)*sizeof(struct request));
		FileArray = (struct request *) malloc(((int)(filesize/buf)+1)*sizeof(struct request));
		
		//while(i < ((int)(fileInfo->st_size/buf)+1)){
		while(i < ((int)(filesize/buf)+1)){
			tmp = getChars(fp,buf, i*buf);
			
			//info in req 
			tmpSize = buf;
			//if((int)(fileInfo->st_size/buf) == i) tmpSize = fileInfo->st_size%buf;
			if((int)(filesize/buf) == i) tmpSize = filesize%buf;
			memcpy(FileArray[i].name,tmp,tmpSize*sizeof(char)); //daten
			FileArray[i].FlNr = tmpSize; //datenlänge
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

