//only some parts ... of the clientApp
#include "ClientSy.h" //SAP to our protocol
//#include "config.h" Wird nicht gebraucht
#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <winsock2.h>
#include "toUdp.h"
#include "file.h"
#include "window.h"
#include "userinformation.h"
//#include "data.h"

void Usage(char *ProgName){ //How to use program
	fprintf(stderr, "\nKommandoparameter:\n\n");
	fprintf(stderr, "[-l xxx]\tZuf\x84lliger Paketverlust in Prozent. (Standard: 0=OFF).\n");
	fprintf(stderr, "[-p xxx]\tPort f\x81 \br Socket bind. (Standard: %s)\n",DEFAULT_PORT);	
	exit(1);
}

int main( int argc, char *argv[]){
			long i;
			int Paketverlust = 0,PaketverlustProzent = DEFAULT_FAILURE;
			struct request erstverbindung, *paket, *buff,paketcpy;
			struct answer antwort;
			int verbindungBeendet = 0, ackWindow;
			clock_t timer;
			FILE *fp=NULL;
			char *PaketverlustProzenttmp = "",*PortTmp = DEFAULT_PORT;

			struct window	*fensterArray = NULL;
			struct request	*fileArray = NULL;
			unsigned int fileArraySize = 0;
			int zufallszahl;

			//zufallszahl für paketverlust
			srand(time(0));

			//Eingabeparameter überprüfen
			if (argc > 1) {
				for (i = 1; i < argc; i++) {
					if (((argv[i][0] == '-' ) || (argv[i][0] == '/' )) && (argv[i][1] != 0) && (argv[i][2] == 0)) {
						switch (tolower(argv[i][1])) {
							//paketverlust
							case 'l':	if (argv[i + 1]){
											if (argv[i + 1][0] != '-' ){
												Paketverlust = 1;
												PaketverlustProzenttmp = argv[++i];
												break;
											}
										}
							//port
							case 'p':	if (argv[i + 1]){
											if (argv[i + 1][0] != '-' ){
												PortTmp = argv[++i];
												break;
											}
										}
							default:	Usage(argv[0]);
										break;
						}
					
					}//else Usage(argv[0]);
				}
			}

			//socket registrieren
			initSocket(PortTmp);
			
			//paketverlust überschreiben
			if(strlen(PaketverlustProzenttmp)>0) PaketverlustProzent = atoi(PaketverlustProzenttmp);
			printf("\nWarte auf Request... ");
			//auf verbindung warten
			memcpy(&erstverbindung,getRequest(),sizeof(struct request));
			
			UserInformation(2,&erstverbindung,NULL);
			printf("empfangen, starte Datenaustausch.\n");

			//req prüfen
			if(erstverbindung.ReqType == ReqHello && erstverbindung.FlNr >= 1 && erstverbindung.FlNr <= 10 && erstverbindung.SeNr > 0){
				//datenarray erstellen
				fileArray = createFileArray(erstverbindung.SeNr);
				//fensterarray erstellen
				fensterArray = createWindowArray(erstverbindung.FlNr);
				//antwort füllen
				antwort.AnswType = AnswHello;
				//printf("\nSende Antwort (AnswHello)...\n");
				antwort.SeNo = erstverbindung.SeNr; //filesize bestätigen
				antwort.FlNr = erstverbindung.FlNr; //fenstergröße bestätigen
				//fenstergröße zwischenspeichern
				fileArraySize = erstverbindung.SeNr;
			}else{
				//antwort mit error code füllen
				antwort.AnswType = AnswWarn;
				antwort.SeNo = 4l; //error nr 4
				antwort.FlNr = 0l; //empty
			}

			//ack verschicken
			sendAnswer(&antwort);
			
			//Antwort auf Bildschirm ausgeben
			UserInformation(3,0,&antwort);
			
			//schleife
			do {
				//warte auf nachricht
				paket = getRequest();

				//empfangenes Paket zwischenspeichern in struct request variable
				memcpy(&paketcpy,paket,sizeof(struct request));

				//Informationen zum Paket ausgeben
				UserInformation(4,&paketcpy,0);

				//Pointer wird "zerstört", zurückkopieren aus zwischengespeicherten Paket
				memcpy(paket,&paketcpy,sizeof(struct request));
				//if paketverlust true
				zufallszahl= rand();
				if(Paketverlust != 0 && (zufallszahl % 100) <= PaketverlustProzent){					//überspringen und keine ack schicken
					printf("Paket verloren!\n");
					//überspringen und keine ack schicken
					continue;
				//else
				}else{
					//daten prüfen 
					if(paket->ReqType == ReqData && paket->SeNr < fileArraySize){
						//daten speichern
						memcpy(&fileArray[paket->SeNr],paket,sizeof(struct request));
						//quittung markieren
						getNextFreeWindow(fensterArray,paket->SeNr,AnswOk,-1);
					}else if(paket->ReqType == ReqClose){
						getNextFreeWindow(fensterArray,paket->SeNr,AnswClose,-1);
					}else{
						getNextFreeWindow(fensterArray,paket->SeNr,AnswErr,4);
					}

					
					if((ackWindow = getWindowWithAck(fensterArray)) == -1) break; //window mit ack holen
					
					//auf ack reagieren
					if(fensterArray[ackWindow].AnswType == AnswOk){
						antwort.AnswType = fensterArray[ackWindow].AnswType;	
						antwort.SeNo = fensterArray[ackWindow].SqNr;	
					//ist verbindung beendet?
					}else if(fensterArray[ackWindow].AnswType == AnswClose){
						antwort.AnswType = fensterArray[ackWindow].AnswType;
						antwort.SeNo = fensterArray[ackWindow].SqNr;
						
					}else{
						antwort.AnswType = fensterArray[ackWindow].AnswType;
						antwort.SeNo = fensterArray[ackWindow].error;
					}
					//Antwort senden und ausgeben
					sendAnswer(&antwort);
					UserInformation(3,NULL,&antwort);
					if(antwort.AnswType == AnswClose) verbindungBeendet = 1;
				}
			//solange die verbindung nicht beendet wurde
			}while(!verbindungBeendet);

			//socket freigeben
			exitSocket();
			
			//datei erstellen

			if (fp=openFile(erstverbindung.fname,&fp)) {
				printf("\nDatei wird gespeichert...\n");
				//datei zusammensetzen und speichern
				if (saveFile(fp,fileArray,(fileArraySize/PufferSize)))  {
					closeFile(fp);
					printf("Datei \x81 \bbermittelt und gespeichert.\nBeende...\n");
					return EXIT_SUCCESS;
					}
				} 
			//filepointer schließen, falls speichern nicht funktioniert hat
			if(fp != NULL) closeFile(fp);	
			return EXIT_FAILURE;
			
}