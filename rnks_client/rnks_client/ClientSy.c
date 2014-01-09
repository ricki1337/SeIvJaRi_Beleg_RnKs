#include "ClientSy.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include "data.h"
#include "toUdp.h"

#include <ctype.h>

/*static struct request req;*/


unsigned short portnr = 50000;
char *server_port= "50000";

SOCKET ConnSocket;
SOCKET DataSocket;
SOCKET mysocket;

/* Declare socket address "local" as static */
static struct sockaddr_in6 localAddr;
struct sockaddr *sockaddr_ip6_local = NULL;

/*Declare socket address "remote" as static*/
static struct sockaddr_in6 remoteAddr;
struct sockaddr sockaddr_ip6_remote;
SOCKADDR_STORAGE from;



void initConnection(char *serverIP,char *port, char* file, char* fenstergroesse){
	struct sockaddr_in6 serversocket;
	struct request nachricht;
	char* reqChar;
	int retSend, reqLen,test;
	unsigned long addr;

	WSADATA wsaData;
	WORD wVersionRequested;
	
	wVersionRequested = MAKEWORD(2,1);
	if( WSAStartup( wVersionRequested,&wsaData ) == SOCKET_ERROR ){
		printf( "Error: WSAStartup() throws error nr. %d!\n" ,WSAGetLastError());
		exit(-1);
	}

	
	memset( &serversocket, 0, sizeof (serversocket));

	//socket wird vom os geholt
	mysocket = socket(AF_INET6,SOCK_DGRAM,0);
	if(mysocket < 0){//im fehlerfall
		printf("Error: socket() throws error nr. %d!\n",WSAGetLastError());
		exit(-1); //ende!
	}
	
	
	serversocket.sin6_family = AF_INET6; //protokoll setzen
		
	//ip adresse setzen
	if ((addr = inet_addr( serverIP )) != INADDR_NONE) {
       /* serverIP ist eine numerische IP-Adresse. */
       memcpy( (char *)&serversocket.sin6_addr, &addr, sizeof(addr));
   }
	//TODO: DNS?
	
	//fallback wenn serverIP == NULL -> localhost!
	if( serverIP == NULL) serversocket.sin6_addr = in6addr_loopback;

	
	serversocket.sin6_port = htons(atoi(port)); //port setzen
	
	
	//stelle verbindung her
	if(connect(mysocket,(struct sockaddr*)&serversocket,sizeof(serversocket)) < 0){//im fehlerfall
		printf("Error: connect() throws error nr. %d!\n",WSAGetLastError());
		WSACleanup ();
		exit(-1);
	}

	//schicke erste nachricht -- entfällt!
	nachricht.FlNr = (long) fenstergroesse; //fenstergröße
	if(file != NULL) memcpy(nachricht.fname,file,sizeof(nachricht.fname));//dateiname
	nachricht.ReqType = ReqHello;//nachrichtentyp
	nachricht.SeNr = 0;

	//umwandeln des struct request in char*
	reqChar = (char*) malloc(sizeof(nachricht));
	memcpy(reqChar,&nachricht,sizeof(nachricht));

	//senden der ersten nachricht
	retSend = sendto(mysocket,reqChar,sizeof(nachricht),0,(struct sockaddr*)&serversocket,sizeof(serversocket));

	//auswerten des returnwertes von send
	if(retSend != sizeof(nachricht)){
		printf("Error: Es wurden nicht alle Daten versand!"); //TODO!
	}
}

int initSocket(){
	int b;
	int val,i=0;
	int addr_len;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	WSADATA wsaData;
	WORD wVersionRequested;
	
	wVersionRequested = MAKEWORD(2,1);
	if( WSAStartup( wVersionRequested,&wsaData ) == SOCKET_ERROR ){
		printf( "SERVER: WSAStartup() failed!\n" );
		printf( " error code: %d\n" ,WSAGetLastError());
		exit(-1);
	}

	ConnSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	
	if (ConnSocket <0 ) {
		fprintf(stderr, "Client: Error Opening socket: Error %d\n" ,WSAGetLastError());
		WSACleanup ();
		exit( -1);
	}
	
	ZeroMemory( &hints,sizeof(hints) );
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	//Initialize address
	
	hints.ai_flags = AI_PASSIVE; //localhost
	
	val = getaddrinfo(NULL, server_port, &hints, &result);
	
	if(val != 0){
		printf( "getaddrinfo failed with error: %d\n" , val);
		WSACleanup();
		exit(-1);
	}
	
	printf( "getaddrinfo returned success\n" );
	
	//Retrieve the address
	
	for (ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
		printf( "getaddrinfo response %d\n" , i++);
		printf( "\tFlags: 0x%x\n" , ptr->ai_flags);
		printf( "\tFamily: ");
		switch (ptr->ai_family) {
			case AF_UNSPEC:	printf( "Unspecified\n" );
							break;
			case AF_INET:	printf( "AF_INET (IPv4)\n" );
							break;
			case AF_INET6:	printf( "AF_INET6 (IPv6)\n" );
							sockaddr_ip6_local = (struct sockaddr *) ptr->ai_addr;
							addr_len= ptr->ai_addrlen;
							break;
			default :		printf("Other %ld\n" , ptr->ai_family);
							break;
		}

		// Bind socket to host address
		printf( "in bind\n" );
		b = bind(ConnSocket, sockaddr_ip6_local, addr_len);
		if (b == SOCKET_ERROR) {
			fprintf(stderr, "bind() failed: error %d\n" ,WSAGetLastError());
			WSACleanup();
			exit (-1);
		}
	}
	
	freeaddrinfo(result);
	return (0);
}

struct request* getRequest() {
	static long seq_number = 0;
	int recvcc; /* Length of message */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	struct request req;
	char* tmp;
	/* Receive a message from a socket */
	printf( "vor recvfrom\n" );
	tmp = (char *) malloc(sizeof(struct request));
	recvcc = recvfrom(ConnSocket, tmp, sizeof(struct request), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize);
	memcpy(&req,tmp,sizeof(struct request));
	//req = (struct request) tmp;
	sizeof(req);
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
	
	printRequest(&req);

	return (&req);
}


void sendAnswer(struct answer *answ) {
	static int i = 0; //dirty ...
	char * reqChar;
	int w;
	/*i++;
	
	if ((i%3)==0){
		printf( "PACKET ACK WILL BE DELEYED by 500ms seqNr %d\n\n",answ->SeNo);
		Sleep(500); // or just simply skip the sendto to simulate dropping of packet
	}
	*/
	printAnswer(answ);
	
	//reqChar = (char*) malloc(sizeof(answ));
	//memcpy(reqChar,&answ,sizeof(answ));


	w = sendto(ConnSocket, (char * ) answ, sizeof(*answ), 0, ( struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
	
	if (w == SOCKET_ERROR) {
		fprintf(stderr, "send() failed: error %d\n",WSAGetLastError());
	}
}

void configSocket(){
	if(!setsockopt(ConnSocket,SOL_SOCKET,SO_SNDTIMEO,(const char *)'1',sizeof(char))){ //ändere socketoption, sodass der timeout von recvfrom bei 1 ms zuschlägt
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