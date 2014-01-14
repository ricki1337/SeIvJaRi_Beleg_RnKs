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

u_long blocking = 1;



//Socket init
int initSocket(char * port){
	int b;
	int val,i=0;
	int addr_len;
	
	/*define for IPv6 Address convert
	LPSOCKADDR sockaddr_ip;
	TCHAR ipstringbuffer[46];
    DWORD ipbufferlength = 46;
	INT iRetval;*/

	/*  addrinfo structure is used by the getaddrinfo function to hold host address information.*/
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	WSADATA wsaData;
	WORD wVersionRequested;
	
	wVersionRequested = MAKEWORD(2,1);
	if( WSAStartup( wVersionRequested,&wsaData ) == SOCKET_ERROR ){
		printf( "SERVER: WSAStartup() failed!\n" );
		printf( " error code: %d\n" ,WSAGetLastError());
		exit(-1);
	}

	/*socket function creates a socket that is bound to a specific transport service provider
	AF_INET6   -> Internet Protocol version 6 (IPv6)
	SOCK_DGRAM -> Socket supports User datagram protocol
	IPPROTO_UDP-> use User Datagram Protocol (UDP) */

	ConnSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	
	if (ConnSocket <0 ) {
		fprintf(stderr, "Client: Error Opening socket: Error %d\n" ,WSAGetLastError());
		WSACleanup ();
		exit( -1);
	}

	
	/*hints is an optional pointer to a struct addrinfo, as defined by <netdb.h> ...
	This structure can be used to provide hints concerning the type of socket that the caller supports or wishes to use*/
	ZeroMemory( &hints,sizeof(hints) );
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	//Initialize address
	
	hints.ai_flags = AI_PASSIVE; //localhost
	
	//val = getaddrinfo(NULL, server_port, &hints, &result);
	val = getaddrinfo(NULL, port, &hints, &result);
	
	if(val != 0){
		printf( "getaddrinfo failed with error: %d\n" , val);
		WSACleanup();
		exit(-1);
	}
	printf( "getaddrinfo returned success\n" );
	
	//Retrieve the address?
	
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
 
							/*// We use WSAAddressToString since it is supported on Windows XP and later
							// The buffer length is changed by each call to WSAAddresstoString
							// So we need to set it for each iteration through the loop for safety
							ipbufferlength = 46;
							iRetval = WSAAddressToString(sockaddr_ip6_local, (DWORD) ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength );
							if (iRetval)
								printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
							else    
								printf("\tIPv6 address: %s\n", ipstringbuffer);*/

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
	
	printf("\nWait for message from socket...");
	recvcc = recvfrom(ConnSocket, tmp, sizeof(struct request), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize);
	printf("\tpacket received, size: %d\n",recvcc);
	memcpy(&req,tmp,sizeof(struct request));
	
	if(WSAGetLastError() == 10035) return NULL;

	if (recvcc == SOCKET_ERROR) {
		fprintf(stderr, "receive from socket failed: error %d\n" ,WSAGetLastError());
		closesocket(ConnSocket);
		WSACleanup();
		exit(-1);
	}
	
	if (recvcc == 0){
		printf("Sender closed connection\n" );
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
	ioctlsocket(ConnSocket,FIONBIO,&blocking);
	if(!setsockopt(ConnSocket,SOL_SOCKET,SO_SNDTIMEO,(const char *)'1',sizeof(char))){ //ändere socketoption, sodass der timeout von recvfrom bei 1 ms zuschlägt
		printf("Error! setsockopt throws error nr. %d", WSAGetLastError());
		closesocket(ConnSocket);
		WSACleanup ();
		exit(-1);
	}
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
	printf("##Request from Sender\n");
	printf("\tRequest Type: \t%c\n",req->ReqType);
	printf("\tSequenz no.: \t%d\n",req->SeNr);
	printf("\tFile no.: \t%d\n",req->FlNr);
	printf("\tFilename: \t%s\n",req->fname);
	printf("\tname: \t%s\n",req->name);
	printf("\n\n");
}
void printAnswer(struct answer *ans){
	printf("\n\n");
	printf("##Answer to Sender\n");
	printf("\tAnswer Type: \t%c\n",ans->AnswType);
	printf("\tSequenz no.: \t%d\n",ans->SeNo);
	printf("\tFile no.: \t%d\n",ans->FlNr);
	printf("\n\n");
}