#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>

/*static struct request req;*/
struct request req;

unsigned short portnr = 50000;
char *server_port= "50000";

SOCKET ConnSocket;

/* Declare socket address "local" as static */
static struct sockaddr_in6 localAddr;
struct sockaddr *sockaddr_ip6_local = NULL;

/*Declare socket address "remote" as static*/
static struct sockaddr_in6 remoteAddr;
struct sockaddr sockaddr_ip6_remote;
SOCKADDR_STORAGE from;


int initServer(){
	int b;
	int val,i=0;
	int addr_len;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	WSADATA wsaData;
	WORD wVersionRequested;
	
	wVersionRequested = MAKEWORD(2,1);
	if( WSAStartup( wVersionRequested,&wsaData ) == SOCKET_ERROR ){
		printf( "SERVER: WSAStartup() failed!\n" );
		printf( " error code: %d\n" ,WSAGetlastError());
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
			fprintf(stderr, "bind() failed: error %d\n" ,WSAGetlastError());
			WSACleanup();
			exit (-1);
		}
	}
	
	freeaddrinfo(result);
	return (0);
}

struct request *getRequest() {
	static long seq_number = 0;
	int recvcc; /* Length of message */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	
	/* Receive a message from a socket */
	printf( "vor recvfrom\n" );

	recvcc = recvfrom(ConnSocket, (char *)&req, sizeof (req), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize);

	if (recvcc == SOCKET_ERROR) {
		fprintf(stderr, "recv() failed: error %d\n" ,WSAGetlastError());
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
	return (&req);
}

void sendAnswer(struct answer *answ) {
	static int i = e; //dirty ...

	int w;
	i++;
	
	if ((i%3)==0){
		printf( "PACKET ACK WILL BE DELEYED by 500ms seqNr %d\n\n",answ->SeNo);
		Sleep(500); // or just simply skip the sendto to simulate dropping of packet
	}
	
	w = sendto(ConnSocket, ( const char *)answ, sizeof(*answ), e, ( struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
	
	if (w == SOCKET_ERROR) {
		fprintf(stderr, "send() failed: error %d\n",WSAGetLastError());
	}
}

int exitServer() {
	
	closesocket(ConnSocket);
	printf( "in exit server\n" );
	
	if (WSACleanup()==SOCKET_ERROR){
		printf( "SERVER: WSACleanup() failed!\n" );
		printf( " error code: %d\n",WSAGetLastError());
		exit(-1);
	}
	return(0);
}