
#include "serverSy.h" //SAP to our protocol
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>
#include "window.h"
#include "file.h"
#include <time.h>
#include "Timer.h"
#include "toUdp.h"
#include "data.h"

void Usage(char *ProgName){ //How to use program
	fprintf(stderr, P_MESSAGE_1);
	fprintf(stderr, P_MESSAGE_6, ProgName);
	fprintf(stderr, P_MESSAGE_7, (DEFAULT_SERVER == NULL) ? "loopback address" : DEFAULT_SERVER);
	fprintf(stderr, P_MESSAGE_8, DEFAULT_PORT);
	fprintf(stderr, P_MESSAGE_9);
	exit(1);
}
/*
int printAnswer(struct answer *answPtr){
	
	switch (answPtr->AnswType) {
		case AnswHello:	printf( "Answer Hello" );
						break;
		case AnswOk:	printf( "Answer Ok Packet acknowledged No: %u ", answPtr->SeNo);
						break;
		case AnswClose:	printf( "Answer Close" );
						break;
		case AnswWarn:	printf( "Answer Warning: %s" ,errorTable[answPtr->ErrNo]);
						break;
		case AnswErr:	printf( "Answer Error: %s" ,errorTable[answPtr->ErrNo]);
						break;
		default:		printf( "Illegal Answer" );
						break;
		}; 
		
		puts( "\n");
		return answPtr->AnswType;
}*/

int main(int argc, char* argv[]){

	
	long i;

	struct answer	*erstverbindung = NULL, *SqAnswer;
	struct request	fehlermeldung_erstverbindung;
	struct window	*fensterArray = NULL;
	struct request	*FileArray = NULL;
	struct timeouts *timerArray = NULL;
	int				timeoutRequest;
	int				aktuellesFile = 0, maxFiles=0;
	int				aktuellesWindow = 0;

	int reihenfolgeVertauschen = 0;
	int Paketverlust = 0;
	char *Empfaenger = DEFAULT_SERVER;
	char *Filename = "";
	char *Port = DEFAULT_PORT;
	//Default Window Size -> prog argument
	char *tmpWindow_size = "";
	int Window_size;
	unsigned char wa;
	int tmp=0, verbindungBeenden=0;

	float timer;

	//argumente auswerten
	if (argc > 1) {
				for (i = 1; i < argc; i++) {
					if (((argv[i][0] == '-' ) || (argv[i][0] == '/' )) && (argv[i][1] != 0) && (argv[i][2] == 0)) {
						switch (tolower(argv[i][1])) {
							//empfänger Adresse
							case 'a':	if(argv[i + 1]){
											if(argv[i + 1][0] != '-' ){
												Empfaenger= argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							//empfänger Port
							case 'p':	if(argv[i + 1]){
											if(argv[i + 1][0] != '-' ){
												Port = argv[++i];
												break;
											}			
										}
										Usage(argv[0]);
										break;
							//FileName
							case 'f':	if (argv[i + 1]){
											if (argv[i + 1][0] != '-' ) {
												Filename = argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							//Window Size
							case 'w':	if (argv[i + 1]){
											if (argv[i + 1][0] != '-' ){
												tmpWindow_size = argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							//reihenfolgeVertauschen
							case 'r':	reihenfolgeVertauschen = 1;
										break;
							default:	Usage(argv[0]);
										break;
						}
					
					}else Usage(argv[0]);
				}
			}
	//fenstergröße setzen
	Window_size = DEFAULT_WINDOW;
	if(tmpWindow_size != "")
		Window_size = atoi(tmpWindow_size);

	//server starten
	initConnection(Empfaenger,Port,Filename,Window_size);

	//auf antwort vom empfänger warten
	erstverbindung = getAnswer();
	
	//workaround 
	wa = erstverbindung->AnswType;
	maxFiles = erstverbindung->SeNo/(PufferSize-1)+1;
	//antwort prüfen
	//if(erstverbindung->AnswType != AnswHello){//falscher reqtype
	if(wa != AnswHello){//falscher reqtype
		//verbindung beenden
		fehlermeldung_erstverbindung.ReqType = ReqClose;		
		fehlermeldung_erstverbindung.SeNr = 0;
		//info versenden
		sendRequest(&fehlermeldung_erstverbindung);
	}else{ //request erstverbindung korrekt start

		//fenster einrichten
		fensterArray = createWindowArray(Window_size);
		//dateisegment array erstellen
		FileArray = createFileArray(Filename);

		//schleife
		do{
			//senden
			//timer starten
			timer = (float)clock()/CLOCKS_PER_SEC;
			//schleife 
			do{
				//wenn fenster vorhanden
				if(getOpenWindows(fensterArray) < Window_size){
					if((timeoutRequest = getTimeout(timerArray)) != -1){//auf timeouts prüfen
						//timer löschen
						timerArray = del_timer(timerArray,FileArray[timeoutRequest].SeNr);
						//timer einfügen
						timerArray = add_timer(timerArray,TIMEOUT,FileArray[timeoutRequest].SeNr);
						//daten mit timeout nochmals versenden
						sendRequest(&FileArray[timeoutRequest]);
					}else if(aktuellesFile == maxFiles && getTimeout(timerArray) == -1 && verbindungBeenden == 0){
						//es wurden alle info übermittelt
						//baue verbindung ab
						//erstmal nur überspringen
						break;
					}else if(aktuellesFile < maxFiles) {
					//keine abzuarbeiten, neue daten versenden
						//zeitscheibe aktualisieren
						if((aktuellesWindow = getNextFreeWindow(fensterArray,aktuellesFile)) > -1){
							//timer einfügen
							add_timer(timerArray,TIMEOUT,FileArray[aktuellesFile].SeNr);
							//daten senden
							sendRequest(&FileArray[aktuellesFile]);
							//nächstes file
							aktuellesFile++;
						}
					}
				}else{
					printf("Error! Server meldet: %s",errorTable[0]);//sendefenster voll!
					Sleep(TIMEOUT_INT-(((float)clock()/CLOCKS_PER_SEC)-timer));
					//break;//schleife verlassen
				}
			//solange zeit-timer < 200
			}while((((float)clock()/CLOCKS_PER_SEC)-timer) > TIMEOUT_INT && (aktuellesFile-1) <= maxFiles);
			
			//empfangen
			configSocket(); //timeout für recvfrom anpassen
			//timer starten
			timer = (float)clock()/CLOCKS_PER_SEC;
			do{
				//daten empfangen
				SqAnswer = getAnswer();
				//quittungen prüfen und dateisegment aktualisieren
				wa = SqAnswer->AnswType;
				if(wa == AnswOk){ //welche fehler können denn auftauchen?
					setWindowFree(fensterArray,SqAnswer->SeNo);
					del_timer(timerArray,SqAnswer->SeNo);
				}else if(wa == AnswClose){
					verbindungBeenden = 1;
				}
				//timer decrementieren
				decrement_timer(timerArray);
			}while((((float)clock()/CLOCKS_PER_SEC)-timer) > TIMEOUT_INT);
		//solange dateisegment nicht komplett quittiert
		//&& getOpenWindows(fensterArray) != 0
		}while(verbindungBeenden == 0);// && getTimeout(timerArray) != -1 && (aktuellesFile-1) != maxFiles);
	}//request erstverbindung korrekt ende

	
	exitSocket();
	scanf("%d");
	return EXIT_SUCCESS;
}