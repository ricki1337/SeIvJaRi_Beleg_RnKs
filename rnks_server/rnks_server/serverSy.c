#include "serverSy.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include "toUdp.h"
#include "file.h"
#include <ctype.h>
#include <time.h>

/*static struct request req;*/
struct request req;

SOCKET ConnSocket;

/*Declare socket address "remote" as static*/
static struct sockaddr_in6 remoteAddr;


void initConnection(char *empfIP,char *port, char* file, int fenstergroesse){
	struct request nachricht;
	char* reqChar;
	int retSend, reqLen,test;
	unsigned long addr;

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
	//TODO: DNS?
	
	//fallback wenn serverIP == NULL -> localhost!
	if( empfIP == NULL) remoteAddr.sin6_addr = in6addr_loopback;

	
	remoteAddr.sin6_port = htons(atoi(port)); //port setzen
	
	
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
	printRequest(&nachricht);

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
	printRequest(paket);
	reqChar = (char*) malloc(sizeof(struct request));
	memcpy(reqChar,paket,sizeof(struct request));
	sendtostat = sendto(ConnSocket, reqChar,sizeof(struct request),0,(struct sockaddr *) &remoteAddr,sizeof(remoteAddr));
	if (sendtostat == SOCKET_ERROR) {
		fprintf(stderr, "send() failed: error %d\n",WSAGetLastError());
	}
}

struct answer* getAnswer(){
	struct answer req;
	static long seq_number = 0;
	int recvcc; /* Length of message */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	char * reqchar;
	/* Receive a message from a socket */
	printf( "vor recvfrom\n" );
	reqchar = (char *) malloc(sizeof(req));
	recvcc = recvfrom(ConnSocket, reqchar, sizeof (req), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize);
	memcpy(&req,reqchar,sizeof(req));
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

	printAnswer(&req);

	return (&req);
}


void configSocket(){
	if(!setsockopt(ConnSocket,SOL_SOCKET,SO_RCVTIMEO,(const char *)'1',sizeof(char))){ //ändere socketoption, sodass der timeout von recvfrom bei 1 ms zuschlägt
		printf("Error! setsockopt throws error nr. %d", WSAGetLastError());
		closesocket(ConnSocket);
		WSACleanup ();
		exit(-1);
	}
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

void printRequest(struct request *req){
	printf("\n\n");
	printf("\t##Request\n");
	printf("ReqType: %c\n",req->ReqType);
	printf("SqNr: %d\n",req->SeNr);
	printf("FlNr: %d\n",req->FlNr);
	printf("fname: %s\n",req->fname);
	printf("name: %s\n",req->name);
	printf("\n\n");
}
void printAnswer(struct answer *ans){
	printf("\n\n");
	printf("\t##Answer\n");
	printf("AnswType: %c\n",ans->AnswType);
	printf("SqNr: %d\n",ans->SeNo);
	printf("FlNr: %d\n",ans->FlNr);
	printf("\n\n");
}