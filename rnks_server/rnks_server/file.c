#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include "data.h"
/*
	- �ffnen der file im textmodus
	- zerteilen der file in ein array a PufferSize-1 + 50-1 zeichen
	- schlie�en der file
*/

struct stat *fileInfo;


FILE* openFile(char *fileName){
	FILE *fp;
	fp = fopen(fileName,"r"); //datei zum lesen �ffnen
	if(fp == NULL) return 0; //datei existiert nicht oder konnte nicht ge�ffnet werden
	fileInfo = (struct stat *)malloc(sizeof(struct stat));
	stat(fileName,fileInfo);//hole info zur datei
	
	return fp;
}

int getFileSize(){
	return fileInfo->st_size;
}

char* getChars(FILE *fp,int anzZeichen, int pos){
	int anzLsbZeichen=0;
	char* buff;
	anzZeichen--;//um 1 decrementieren um \0 anzuf�gen
	if(fp != NULL){ //ist dateizeiger ge�ffnet?
		if(pos > fileInfo->st_size) return NULL;//existiert die gesuchte pos?
		if((anzLsbZeichen = fileInfo->st_size-(pos)) < anzZeichen ) anzZeichen = anzLsbZeichen;//wie viele zeichen k�nnen �berhaupt noch gelesen werden?
			
		buff = (char*) malloc((anzZeichen+1)*sizeof(char));//puffer anpassen
		//lese die zeichen...
		fseek(fp,pos,SEEK_SET);//gehe an die richtige pos
		fread(buff,sizeof(char),anzZeichen,fp);//lese
		buff[anzZeichen] = '\0'; //stringende anf�gen
		return buff;//gib den char* zur�ck
		
	}
	return NULL;
}

void closeFile(FILE *fp){
	free(fileInfo);
	if(fp != NULL) fclose(fp);
}

struct request* createFileArray(char *fileName){
	int i=0,buf,tmpSize; //faktor f�r pos berechnung
	struct request *FileArray;
	char *tmp;
	FILE *fp=NULL;
	
	buf = (int)PufferSize;
	if(fp = openFile(fileName)){

		FileArray = (struct request *) malloc(((int)(fileInfo->st_size/buf)+1)*sizeof(struct request));

		while(i < ((int)(fileInfo->st_size/buf)+1)){
			tmp = getChars(fp,buf, i*buf);
			
			//info in req 
			tmpSize = buf;
			if((int)(fileInfo->st_size/buf) == i) tmpSize = fileInfo->st_size%buf;
			memcpy(FileArray[i].name,tmp,tmpSize*sizeof(char)); //daten
			FileArray[i].FlNr = tmpSize; //datenl�nge
			FileArray[i].ReqType = ReqData; //reqtype
			FileArray[i].SeNr = i;//sqnr
			memcpy(FileArray[i].fname,fileName,strlen(fileName)+1);
			FileArray[i].fname[strlen(fileName)] = '\0';
			i++;
		}
		closeFile(fp);
		return FileArray;
	}
}

