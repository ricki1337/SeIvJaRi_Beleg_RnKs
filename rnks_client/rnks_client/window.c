#include "window.h"
#include <stdio.h>
#include <stdlib.h>


int windowSize;


struct window* createWindowArray(unsigned int windows){
	unsigned int i;
	struct window *newWindowArray;
	windowSize = windows;
	newWindowArray = (struct window*) malloc(windows*sizeof(struct window));
	for(i=0;i<windows;i++){//setzen der default werte
		newWindowArray[i].ack = 0;
		newWindowArray[i].SqNr = -1;//-1 => keine SqNr vergeben!
		newWindowArray[i].AnswType = ' ';
		newWindowArray[i].error = -1;
	}
	return newWindowArray;
}

int getOpenWindows(struct window* windowArray){
	int i,OpenWindows=0;

	for(i=0;i<windowSize;i++)
		if(!windowArray[i].ack)
			OpenWindows++;

	return OpenWindows;	
}

int getNextFreeWindow(struct window* windowArray, unsigned long SqNr, unsigned char AnswType, long error){
	int i;
	for(i=0;i<windowSize;i++){
		if(windowArray[i].ack == 0){
			windowArray[i].ack = 1;
			windowArray[i].SqNr = SqNr;
			windowArray[i].AnswType = AnswType;
			windowArray[i].error = error;
			return i;
		}
	}

	return -1; //error! Keine freien Fenster vorhanden!
}

int setWindowFree(struct window* windowArray, unsigned long SqNr){
	int i;
	for(i=0;i<windowSize;i++){
		if(windowArray[i].SqNr == SqNr){
			windowArray[i].ack = 0;
			windowArray[i].SqNr = -1;
			return 1;
		}
	}
	return 0; //error! SqNr nicht vorhanden!
}

int getWindowWithAck(struct window* windowArray){
	int i;
	for(i=0;i<windowSize;i++){
		if(windowArray[i].ack == 1){
			windowArray[i].ack = 0;
			return i;
		}
	}
	return -1; //error! keine ack vorhanden!
}