#include "debug.h"
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

//#define DEBUG

void Usage(char *ProgName){ //How to use program
	fprintf(stderr, P_MESSAGE_1);
	fprintf(stderr, P_MESSAGE_6, ProgName);
	fprintf(stderr, P_MESSAGE_7, (DEFAULT_SERVER == NULL) ? "loopback address" : DEFAULT_SERVER);
	fprintf(stderr, P_MESSAGE_8, DEFAULT_PORT);
	fprintf(stderr, P_MESSAGE_9);
	exit(1);
}

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
	int	filearraysize; //Anzahl der Pakete,wird ben�tigt um Paketreihenfolge zu vertauschen, wird in createFilearry definiert
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
	int verbindungBeenden=0;

	clock_t timer;

	FILE *f;
	int Portint;
	int l�nge,zeichen=0,nullen_aufgef�llt=0, doppelpunktz�hlerz�hler=0;

	srand(time(NULL)); //seeden des randomgenerators
//	 Reihenfolgevertauschen, UserInfo

	//argumente auswerten
	if (argc > 1) {
				for (i = 1; i < argc; i++) {
					if (((argv[i][0] == '-' ) || (argv[i][0] == '/' )) && (argv[i][1] != 0) && (argv[i][2] == 0)) {
						switch (tolower(argv[i][1])) {
							
							//empf�nger Adresse
							case 'a':	if(argv[i + 1]){
											if(argv[i + 1][0] != '-' ){
												Empfaenger= argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							//empf�nger Port
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
	
//Portnummer pr�fen
	Portint = atoi(Port); 
	if (Portint<49152 || Portint>65535){
		fprintf(stderr, P_MESSAGE_11);
		exit(1);}

	//pr�fen ob txt Datei existiert
    f = fopen(Filename, "r");
	if (f == NULL){ 
		fprintf(stderr, P_MESSAGE_12);
		exit(1);}

	//ipv6 adressformat pr�fen
	if (Empfaenger == "::" || Empfaenger == "::1" || Empfaenger == NULL )Empfaenger = NULL;
		else{

				l�nge = strlen(Empfaenger);

				if (l�nge > 39){fprintf(stderr, P_MESSAGE_13); exit(1);}

				for (i=0;i<l�nge;i++){
					zeichen++;
					if (Empfaenger[i]==':') zeichen=0, doppelpunktz�hlerz�hler++;

					if (zeichen>4){fprintf(stderr, P_MESSAGE_13); exit(1);}

					if (Empfaenger[i]==':' && Empfaenger[i+1]==':') {nullen_aufgef�llt=1;}
				}

				if (doppelpunktz�hlerz�hler < 7 && nullen_aufgef�llt != 1){fprintf(stderr, P_MESSAGE_13); exit(1);}}

	//fenstergr��e setzen
	Window_size = DEFAULT_WINDOW;
	if(tmpWindow_size != "")
		Window_size = atoi(tmpWindow_size);


	//daten f�r verbindungsabbau vorbereiten
	rverbindungsabbau.ReqType = ReqClose;
	memcpy(rverbindungsabbau.name,Filename,strlen(Filename));
	rverbindungsabbau.name[strlen(Filename)] = '\0';
	rverbindungsabbau.SeNr = 0xFFFF;
	rverbindungsabbau.FlNr = 0;
	rverbindungsabbau.fname[0] = '\0';



	//server starten
	initConnection(Empfaenger,Port,Filename,Window_size);

	//wenn innerhalb von 10sec keine antwort -> beenden

	//auf antwort vom empf�nger warten
	erstverbindung = getAnswer();
	
	//workaround 
	wa = erstverbindung->AnswType;
	maxFiles = (int)(erstverbindung->SeNo/PufferSize)+1;
	//antwort pr�fen
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
		//sendeReihenfolge erstellen in abh�nigkeit von "int reihenfolgeVertauschen"
		sendeReihenfolgearray= (int*) malloc(sizeof(int)*filearraysize);
		sendeReihenfolge(sendeReihenfolgearray,filearraysize,reihenfolgeVertauschen);
		
		//schleife
		do{
			#ifdef DEBUG
				//f�r �bersicht sorgen...
				system("cls");
				//window anzeigen
				showWindow(fensterArray);
			#endif

			//timer starten
			timer = clock();

			//wenn fenster vorhanden oder aber ein timeout auftritt...sonst deadlock!
			if((timeoutRequest = decrement_timer(timerArray)) != -1 || getOpenWindows(fensterArray) < Window_size){
				if(timeoutRequest != -1){//auf timeouts pr�fen
					//workaround f�r closing paket....
					if(timeoutRequest == 0xFFFF){
						//timer l�schen
						timerArray = del_timer(timerArray,timeoutRequest);
						//timer einf�gen
						timerArray = add_timer(timerArray,TIMEOUT,timeoutRequest);
						//daten mit timeout nochmals versenden
						sendRequest(&rverbindungsabbau);
					}else{
						//timer l�schen
						timerArray = del_timer(timerArray,FileArray[timeoutRequest].SeNr);
						//timer einf�gen
						timerArray = add_timer(timerArray,TIMEOUT,FileArray[timeoutRequest].SeNr);
						//daten mit timeout nochmals versenden
						sendRequest(&FileArray[timeoutRequest]);
					}
				}else if(aktuellesFile == maxFiles && timeoutRequest == -1 && verbindungBeenden == 0){
					//es wurden alle info �bermittelt
					//baue verbindung ab
					//erst schauen ob �berhaupt ein fenster frei ist
					if((aktuellesWindow = getNextFreeWindow(fensterArray,0xFFFF)) > -1){
						//timer einf�gen
						timerArray = add_timer(timerArray,TIMEOUT,0xFFFF);
						//daten senden
						sendRequest(&rverbindungsabbau);
					}else{
						printf("Error! Server meldet: %s",errorTable[0]);//sendefenster voll! -> warte einfach mittels select() auf antworten
					}
				}else if(aktuellesFile < maxFiles) {
				//keine timeouts abzuarbeiten, neue daten versenden
					//zeitscheibe aktualisieren
					if((aktuellesWindow = getNextFreeWindow(fensterArray,sendeReihenfolgearray[aktuellesFile])) > -1){
						//timer einf�gen
						timerArray = add_timer(timerArray,TIMEOUT,FileArray[sendeReihenfolgearray[aktuellesFile]].SeNr);
						//daten senden
						sendRequest(&FileArray[sendeReihenfolgearray[aktuellesFile]]);
						//n�chstes file
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
				
				//quittungen pr�fen und dateisegment aktualisieren
				if(SqAnswer.AnswType == AnswOk){ //welche fehler k�nnen denn auftauchen und wie behandelt man sie?
					setWindowFree(fensterArray,SqAnswer.SeNo);
					timerArray = del_timer(timerArray,SqAnswer.SeNo);
				}else if(SqAnswer.AnswType == AnswClose){
					setWindowFree(fensterArray,SqAnswer.SeNo);
					timerArray = del_timer(timerArray,SqAnswer.SeNo);
					verbindungBeenden = 1;//testweise
				}
			
			}

			//warte die restliche zeit
			if(((clock_t)TIMEOUT_INT - (clock()-timer)) > 0) Sleep((clock_t)TIMEOUT_INT - (clock()-timer));

		//solange keine Antwort �ber ReqClose oder noch offene Timeouts oder noch nicht best�tigte Nachrichten
		}while(getTimeout(timerArray) != -1 || getOpenWindows(fensterArray) != 0 || verbindungBeenden == 0);

	}//request erstverbindung korrekt ende

	
	exitSocket();
	
	return EXIT_SUCCESS;
}