#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include "data.h"
/*
	- �ffnen der file im textmodus zum schreiben
	- zusammenf�gen der file aus einem array
	- schlie�en der file
*/

int openFile(char *fileName, FILE *fp){
	fp = fopen(fileName,"w+"); //datei zum schreiben �ffnen
	if(fp == NULL) {
		printf("Error: Could not open file %s!\n",fileName);
			return 0; //datei konnte nicht ge�ffnet werden
		}
	return 1;
}


void closeFile(FILE *fp){
	if(fp != NULL) fclose(fp);
	printf("Succesfull closed file.\n");
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

//Save files from FileArray to local file
int saveFile(FILE *fp, struct request *FileArray, int sizeFA) {
	//char buf[PufferSize];
	int i,c;

	for (i=0;i<sizeFA;i++) {
		//alternativ zeichenwei�e, da m�gliche mehrinhalte beim auff�llen des Puffers fileArray.name?
		//noch zu testen...
		if (c=fputs(FileArray[i].name,fp)==EOF)
			return 0;
		}
	return 1;
}
