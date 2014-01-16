
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

	struct answer	*erstverbindung = NULL, SqAnswer,*SqAnswerTmp = NULL;
	struct request	fehlermeldung_erstverbindung, rverbindungsabbau;
	struct window	*fensterArray = NULL;
	struct request	*FileArray = NULL;
	struct timeouts *timerArray = NULL;
	int				timeoutRequest;
	int				aktuellesFile = 0, maxFiles=0;
	int				aktuellesWindow = 0;
	int	filearraysize; //Anzahl der Pakete,wird benötigt um Paketreihenfolge zu vertauschen, wird in createFilearry definiert
	int *sendeReihenfolgearray;
	int reihenfolgeVertauschen = 0;
	int Paketverlust = 0;
	char *Empfaenger = DEFAULT_SERVER;
	char *EmpfaengerDNS = "";
	char *Filename = "";
	char *Port = DEFAULT_PORT;
	//Default Window Size -> prog argument
	char *tmpWindow_size = "";
	int Window_size;
	unsigned char wa;
	int tmp,tmp1, verbindungBeenden=0,testtimer;

	clock_t timer;

	FILE *f;
	int Portint;
	int länge,zeichen=0,nullen_aufgefüllt=0, doppelpunktzählerzähler=0;

	srand(time(NULL)); //seeden des randomgenerators
//	 Reihenfolgevertauschen, UserInfo

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
			} else Usage(argv[0]); 
	
//Portnummer prüfen
	Portint = atoi(Port); 
	if (Portint<49152 || Portint>65535){
		fprintf(stderr, P_MESSAGE_11);
		exit(1);}

	//prüfen ob txt Datei existiert
    f = fopen(Filename, "r");
	if (f == NULL){ 
		fprintf(stderr, P_MESSAGE_12);
		exit(1);}

	//ipv6 adressformat prüfen
	if (Empfaenger == "::" || Empfaenger == "::1" || Empfaenger == NULL )Empfaenger = NULL;
		else{

				länge = strlen(Empfaenger);

				if (länge > 39){fprintf(stderr, P_MESSAGE_13); exit(1);}

				for (i=0;i<länge;i++){
					zeichen++;
					if (Empfaenger[i]==':') zeichen=0, doppelpunktzählerzähler++;

					if (zeichen>4){fprintf(stderr, P_MESSAGE_13); exit(1);}

					if (Empfaenger[i]==':' && Empfaenger[i+1]==':') {nullen_aufgefüllt=1;}
				}

				if (doppelpunktzählerzähler < 7 && nullen_aufgefüllt != 1){fprintf(stderr, P_MESSAGE_13); exit(1);}}

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
	maxFiles = (int)(erstverbindung->SeNo/PufferSize)+1;
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
		FileArray = createFileArray(Filename,&filearraysize);
		//sendeReihenfolge erstellen in abhänigkeit von "int reihenfolgeVertauschen"
		sendeReihenfolgearray= (int*) malloc(sizeof(int)*filearraysize);
		sendeReihenfolge(sendeReihenfolgearray,filearraysize,reihenfolgeVertauschen);
		
		//schleife
		do{
		
			//timer starten
			timer = clock();
			//wenn fenster vorhanden
			if(getOpenWindows(fensterArray) < Window_size){
				if((timeoutRequest = getTimeout(timerArray)) != -1){//auf timeouts prüfen
					printf("Timeout!#####\n");
					//timer löschen
					timerArray = del_timer(timerArray,FileArray[timeoutRequest].SeNr);
					//timer einfügen
					timerArray = add_timer(timerArray,TIMEOUT,FileArray[timeoutRequest].SeNr);
					//daten mit timeout nochmals versenden
					sendRequest(&FileArray[timeoutRequest]);
				}else if(aktuellesFile == maxFiles && getTimeout(timerArray) == -1 && verbindungBeenden == 0){
					//es wurden alle info übermittelt
					//baue verbindung ab
					//erst schauen ob überhaupt ein fenster frei ist
					if((aktuellesWindow = getNextFreeWindow(fensterArray,0xFFFF)) > -1){
						//timer einfügen
						timerArray = add_timer(timerArray,TIMEOUT,0xFFFF);
						
						//daten vorbereiten
						rverbindungsabbau.ReqType = ReqClose;
						memcpy(rverbindungsabbau.name,Filename,strlen(Filename));
						rverbindungsabbau.name[strlen(Filename)] = '\0';
						rverbindungsabbau.SeNr = 0xFFFF;
						rverbindungsabbau.FlNr = 0;
						rverbindungsabbau.fname[0] = '\0';

						//daten senden
						sendRequest(&rverbindungsabbau);
					}else{
						printf("Error! Server meldet: %s",errorTable[0]);//sendefenster voll! -> warte einfach mittels select() auf antworten
					}
				}else if(aktuellesFile < maxFiles) {
				//keine abzuarbeiten, neue daten versenden
					//zeitscheibe aktualisieren
					if((aktuellesWindow = getNextFreeWindow(fensterArray,sendeReihenfolgearray[aktuellesFile])) > -1){
						//timer einfügen
						timerArray = add_timer(timerArray,TIMEOUT,FileArray[sendeReihenfolgearray[aktuellesFile]].SeNr);
						//daten senden
						sendRequest(&FileArray[sendeReihenfolgearray[aktuellesFile]]);
						//nächstes file
						aktuellesFile++;
					}
				}
			}else{
				printf("Error! Server meldet: %s",errorTable[0]);//sendefenster voll! -> warte einfach mittels select() auf antworten
			}

			//warte auf antwort mittels select()...
			if(antwort_erhalten(timer)){
				SqAnswerTmp = getAnswer();
				//workaround...da sonst falsche daten :(
				SqAnswer.AnswType = SqAnswerTmp->AnswType;
				SqAnswer.FlNr = SqAnswerTmp->FlNr;
				SqAnswer.SeNo = SqAnswerTmp->SeNo;
				
				//quittungen prüfen und dateisegment aktualisieren
				if(SqAnswer.AnswType == AnswOk){ //welche fehler können denn auftauchen und wie behandelt man sie?
					setWindowFree(fensterArray,SqAnswer.SeNo);
					timerArray = del_timer(timerArray,SqAnswer.SeNo);
				}else if(SqAnswer.AnswType == AnswClose){
					setWindowFree(fensterArray,SqAnswer.SeNo);
					timerArray = del_timer(timerArray,SqAnswer.SeNo);
					verbindungBeenden = 1;//testweise
				}
			
			}

			//timer dekrementieren
			decrement_timer(timerArray);

			//warte die restliche zeit
			if(((clock_t)TIMEOUT_INT - (clock()-timer)) > 0) Sleep((clock_t)TIMEOUT_INT - (clock()-timer));

			//setze abbruchbedingung
			//if(SqAnswer.AnswType == AnswClose) verbindungBeenden = 1;
		//solange keine Antwort über ReqClose oder noch offene Timeouts oder noch nicht bestätigte Nachrichten
		}while(getTimeout(timerArray) != -1 || getOpenWindows(fensterArray) != 0 || verbindungBeenden == 0);






		////schleife
		//do{
		//	//senden
		//	//timer starten
		//	timer = clock();

		//	
		//	//schleife 
		//	do{
		//		//wenn fenster vorhanden
		//		if(getOpenWindows(fensterArray) < Window_size){
		//			if((timeoutRequest = getTimeout(timerArray)) != -1){//auf timeouts prüfen
		//				//timer löschen
		//				timerArray = del_timer(timerArray,FileArray[timeoutRequest].SeNr);
		//				//timer einfügen
		//				timerArray = add_timer(timerArray,TIMEOUT,FileArray[timeoutRequest].SeNr);
		//				//daten mit timeout nochmals versenden
		//				sendRequest(&FileArray[timeoutRequest]);
		//			}else if(aktuellesFile == maxFiles && getTimeout(timerArray) == -1 && verbindungBeenden == 0){
		//				//es wurden alle info übermittelt
		//				//baue verbindung ab
		//				//erstmal nur überspringen
		//				break;
		//			}else if(aktuellesFile < maxFiles) {
		//			//keine abzuarbeiten, neue daten versenden
		//				//zeitscheibe aktualisieren
		//				if((aktuellesWindow = getNextFreeWindow(fensterArray,aktuellesFile)) > -1){
		//					//timer einfügen
		//					add_timer(timerArray,TIMEOUT,FileArray[aktuellesFile].SeNr);
		//					//daten senden
		//					sendRequest(&FileArray[aktuellesFile]);
		//					//nächstes file
		//					aktuellesFile++;
		//				}
		//			}
		//		}else{
		//			printf("Error! Server meldet: %s",errorTable[0]);//sendefenster voll!
		//			
		//			Sleep(TIMEOUT_INT-(clock()-timer));
		//			//break;//schleife verlassen
		//		}
		//	//solange zeit-timer < 200
		//	}while((clock()-timer) < (clock_t)TIMEOUT_INT && (aktuellesFile-1) <= maxFiles);
		//	
		//	//empfangen
		//	configSocket(); //timeout für recvfrom anpassen
		//	//timer starten
		//	timer = clock();
		//	do{
		//		printf("Warte auf antwort");
		//		//daten empfangen
		//		SqAnswer = getAnswer();
		//		if (SqAnswer == NULL) break;
		//		printf("habe antwort...");
		//		//quittungen prüfen und dateisegment aktualisieren
		//		wa = SqAnswer->AnswType;
		//		if(wa == AnswOk){ //welche fehler können denn auftauchen?
		//			setWindowFree(fensterArray,SqAnswer->SeNo);
		//			del_timer(timerArray,SqAnswer->SeNo);
		//		}else if(wa == AnswClose){
		//			verbindungBeenden = 1;
		//		}
		//		//timer decrementieren
		//		decrement_timer(timerArray);
		//		printf("clock: %d",clock());
		//			printf("timer: %d",timer);
		//	}while((clock()-timer) < (clock_t)TIMEOUT_INT);
		////solange dateisegment nicht komplett quittiert
		////&& getOpenWindows(fensterArray) != 0
		//}while(verbindungBeenden == 0);// && getTimeout(timerArray) != -1 && (aktuellesFile-1) != maxFiles);
	}//request erstverbindung korrekt ende

	
	exitSocket();
	
	return EXIT_SUCCESS;
}