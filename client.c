//only some parts ... of the clientApp
#include "ClientSy.h" //SAP to our protocol
#include "config.h"
#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysltypes.h>
#include <string.h>
#include <winsock2.h>

void Usage(char *ProgName){ //How to use program
	fprintf(stderr, P_MESSAGE_1);
	fprintf(stderr, P_MESSAGE_6, ProgName);
	fprintf(stderr, P_MESSAGE_7, (DEFAULT_SERVER == NULL) ? "loopback address" : DEFAULT_SERVER);
	fprintf(stderr, P_MESSAGE_8, DEFAULT_PORT);
	fprintf(stderr, P_MESSAGE_9);
	exit(1);
}

int printAnswer(struct answer *answPtr){
	
	switch (answPtr->AnswType) {
		case AnswHello:	printf( "Answer Hello" );
						break;
		case AnswOk:	printf( "Answer Ok Packet acknowledged No: %u " answPtr->SeNo);
						break;
		case AnswClose:	printf( "Answer Close" );
						break;
		case AnswWarn:	printf( "Answer Warning: %s" ,errorTable[answPtr->ErrNo]);
						break;
		case AnswErr:	printf( "Answer Error: %s" ,errorTable[answPtr->ErrNo]);
						break;
		default:		printf( "Illegal Answer" );
						break;
		}; /* switch */
		
		puts( "\n");
		return answPtr->AnswType;
}

int main( int argc, char *argv[])
		{
			//...
			
			long i;
			char *Server = DEFAULT_SERVER;
			char *Filename = "";
			char *Port = DEFAULT PORT;
			//Default Window Size -> prog argument
			char *Window_size = DEFAULT_WINDOW;
			
			FILE *fp;
			//Parameter überprüfen
			if (argc > 1) {
				for (i = 1; i < argc; i++) {
					if (((argv[i][0] == '-' ) || (argv[i][0] == '/' )) && (argv[i][1] != 0) && (argv[i][2] == 0)) {
						switch (tolower(argv[i][1])) {
							//Server Adresse
							case 'a':	if(argv[i + 1]){
											if(argv[i + 1][0] != '-' ){
												Server= argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							//Server Port
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
												Window_size = argv[++i];
												break;
											}
										}
										Usage(argv[0]);
										break;
							default:	Usage(argv[0]);
										break;
						}
					
					}else Usage(argv[0]);
				}
			}
			//...
		}