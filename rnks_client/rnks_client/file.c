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

int openFile(char *fileName, FILE *fp){
	fp = fopen(fileName,"w+"); //datei zum lesen �ffnen
	if(fp == NULL) return 0; //datei existiert nicht oder konnte nicht ge�ffnet werden
	return 1;
}


void closeFile(FILE *fp){
	if(fp != NULL) fclose(fp);
}

struct request* createFileArray(int filesize){
	int i=0; //faktor f�r pos berechnung
	struct request *FileArray;
	
		FileArray = (struct request *) malloc(((int)(filesize/(PufferSize))+1)*sizeof(struct request));
		while(i < ((int)(filesize/(PufferSize))+1)){
			memcpy(FileArray[i].name,"",PufferSize*sizeof(char)); //daten
			FileArray[i].name[0] = '\0';
			FileArray[i].FlNr = 0; //datenl�nge
			FileArray[i].ReqType = 0; //reqtype
			FileArray[i].SeNr = i;//sqnr
			memcpy(FileArray[i].fname,"",sizeof(FileArray[i].fname));
			FileArray[i].fname[0] = '\0';
			i++;
		}
		return FileArray;
}

