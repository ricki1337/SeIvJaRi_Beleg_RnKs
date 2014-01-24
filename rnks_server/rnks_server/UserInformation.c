#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include "data.h"

static int msgcounter=0;

void UserInformation(int MessageCode, struct request *req, struct answer *ans){
#ifndef DEBUG
	switch(MessageCode){
	
	case 1: printf("\n------------Statusinformation---------\n");
			printf("Socket erhalten...Protokoll gesetzt...Port gesetzt...Verbindung wird hergestellt..."); 
			break;

	case 2: printf("\n-----------Sent first request---------\n");
			printf("\t|ReqTyp..............%c\n",req->ReqType);	
			printf("Msg# %d\t|Size................%d\n",msgcounter,req->SeNr );
			printf("\t|Filename............%s\n",req->fname );
			printf("\t|WindowSize..........%d",req->FlNr );
			break;

	case 3: printf("\n-------------Received  reply----------\n");
			printf("\t|AnswType............%c\n",ans->AnswType);
			printf("Msg# %d\t|SqNr................%d\n",msgcounter,ans->SeNo);
			printf("\t|FlNr................%d",ans->FlNr);
			break;

	case 4: printf("\n---------------Sent request-----------\n");
			printf("\t|ReqType.............%c\n",req->ReqType);
			printf("\t|SqNr................%d\n",req->SeNr);
			printf("Msg# %d\t|FlNr................%d\n",msgcounter,req->FlNr);
			printf("\t|Filename............%s\n",req->fname);
			//printf("\t|Content:\n\n\"%s\"\n",req->name);
			break;
	}
	printf("\n\n");
	msgcounter++;
#else
	printf("\nMessage:\n");
	switch(MessageCode){
		case 1: break;
		case 2: printf("inform receiver: req %c\tsize %d\t fname %s\t wsize %d\n\n",req->ReqType,req->SeNr,req->fname,req->FlNr);
				break;
		case 3: printf("rec: ans %c\tsq %d\n",ans->AnswType,ans->SeNo);
				break;
		case 4: printf("sent: req %c\tsq %d\t wsize %d\n",req->ReqType,req->SeNr,req->FlNr);
				break;
		

	}
	printf("\n");
#endif
}
