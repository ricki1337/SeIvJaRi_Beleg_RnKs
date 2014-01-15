#include "serverSy.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include "data.h" //wurde hier auskommentiert da sie schon mit Userinforamtion.c und .h importiert wird. Sonst fehler wegen NeuDefinition
#include "toUdp.h"
#include "file.h"
#include <ctype.h>
#include <time.h>
#include "UserInformation.h"


/*static struct request req;*/
struct request req;

SOCKET ConnSocket;

/*Declare socket address "remote" as static*/
static struct sockaddr_in6 remoteAddr;

int timeoutsocket = 1;
u_long blocking = 1;

void initConnection(char *empfIP,char *port, char* file, int fenstergroesse){
	struct request nachricht;
	char* reqChar;
	int retSend, reqLen,test;
	unsigned long addr;
	struct hostent *host_info;

	WSADATA wsaData;
	WORD wVersionRequested;
	FILE *fp;


	wVersionRequested = MAKEWORD(2,1);
	if( WSAStartup( wVersionRequested,&wsaData ) == SOCKET_ERROR ){
		printf( "Error: WSAStartup() throws error nr. %d!\n" ,WSAGetLastError());
		exit(-1);
	}

	
	memset( &remoteAddr, 0, sizeof (remoteAddr));

	//socket wird vom os geholt
	ConnSocket = socket(AF_INET6,SOCK_DGRAM,0);
	if(ConnSocket < 0){//im fehlerfall
		printf("Error: socket() throws error nr. %d!\n",WSAGetLastError());
		exit(-1); //ende!
	}
	
	
	remoteAddr.sin6_family = AF_INET6; //protokoll setzen
		
	//ip adresse setzen
	if ((addr = inet_addr( empfIP )) != INADDR_NONE) {
       /* serverIP ist eine numerische IP-Adresse. */
       memcpy( (char *)&remoteAddr.sin6_addr, &addr, sizeof(addr));
   }
	
	//fallback wenn serverIP == NULL -> localhost!
	if( empfIP == NULL) 
		remoteAddr.sin6_addr = in6addr_loopback;

	
	remoteAddr.sin6_port = htons(atoi(port)); //port setzen
	
	UserInformation(1,NULL,NULL);
	//stelle verbindung her
	if(connect(ConnSocket,(struct sockaddr*)&remoteAddr,sizeof(remoteAddr)) < 0){//im fehlerfall
		printf("Error: connect() throws error nr. %d!\n",WSAGetLastError());
		WSACleanup ();
		exit(-1);
	}

	//schicke erste nachricht
	nachricht.FlNr = (long) fenstergroesse; //fenstergröße
	if(file != NULL) memcpy(nachricht.fname,file,sizeof(nachricht.fname));//dateiname
	nachricht.fname[strlen(file)] = '\0';
	nachricht.ReqType = ReqHello;//nachrichtentyp
	nachricht.name[0] = '\0';

	fp = openFile(file);
	nachricht.SeNr = getFileSize();//größe der datei
	closeFile(fp);
	
	//umwandeln des struct request in char*
	reqChar = (char*) malloc(sizeof(nachricht));
	memcpy(reqChar,&nachricht,sizeof(nachricht));
	sizeof(nachricht);
	UserInformation(2,&nachricht,NULL);

	//senden der ersten nachricht
	retSend = sendto(ConnSocket,reqChar,sizeof(nachricht),0,(struct sockaddr*)&remoteAddr,sizeof(remoteAddr));

	//auswerten des returnwertes von send
	if(retSend != sizeof(nachricht)){
		printf("Error: Es wurden nicht alle Daten versand!"); //TODO!
	}
}


void sendRequest(struct request *paket){
	int sendtostat;
	char* reqChar;

	reqChar = (char*) malloc(sizeof(struct request));
	memcpy(reqChar,paket,sizeof(struct request));
	sendtostat = sendto(ConnSocket, reqChar,sizeof(struct request),0,(struct sockaddr *) &remoteAddr,sizeof(remoteAddr));
	if (sendtostat == SOCKET_ERROR) {
		fprintf(stderr, "send() failed: error %d\n",WSAGetLastError());
	}
	UserInformation(4,paket,NULL);
}

struct answer* getAnswer(){
	struct answer req;
	static long seq_number = 0;
	int recvcc; /* Length of message */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	char * reqchar;
	/* Receive a message from a socket */
	
	reqchar = (char *) malloc(sizeof(req));
	recvcc = recvfrom(ConnSocket, reqchar, sizeof (req), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize);
	memcpy(&req,reqchar,sizeof(req));

	if(WSAGetLastError() == 10035) return NULL;

	if (recvcc == SOCKET_ERROR) {
		fprintf(stderr, "recv() failed: error %d\n" ,WSAGetLastError());
		closesocket(ConnSocket);
		WSACleanup();
		exit(-1);
	}
	
	if (recvcc == 0){
		printf("Client closed connection\n" );
		closesocket(ConnSocket);
		WSACleanup ();
		exit(-1);
	}

	UserInformation(3,NULL,&req);

	return (&req);
}

int exitSocket() {
	
	closesocket(ConnSocket);
	printf( "in exit server\n" );
	
	if (WSACleanup()==SOCKET_ERROR){
		printf( "SERVER: WSACleanup() failed!\n" );
		printf( " error code: %d\n",WSAGetLastError());
		exit(-1);
	}
	return(0);
}

int antwort_erhalten(clock_t timer){
	return select(ConnSocket,(fd_set*)ConnSocket,NULL,NULL,(struct timeval *)(TIMEOUT_INT - (clock()-timer)));
}

void sendeReihenfolge(int *array,int size,int unordnung){
	int i=0;
	
	for(i=0;i<size;i++){//array sortiert füllen
		array[i]=i;
	}
	
	if(unordnung){
	//array in zufällige reihfolge bringen
		for(i=0;i<size;i++){
				int xrand=rand() % size; //Wertebereich des array
				int tmp;
		
				tmp=array[xrand];

				array[xrand]=array[i];
				array[i]=tmp;
			}
	}
}