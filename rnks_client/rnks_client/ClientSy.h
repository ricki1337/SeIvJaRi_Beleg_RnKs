#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"

#include <ctype.h>

int initSocket(char * port);
struct request *getRequest();
void sendAnswer(struct answer *answ);
void configSocket();
int exitSocket();
void printRequest(struct request *req);
void printAnswer(struct answer *ans);