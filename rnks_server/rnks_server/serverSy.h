#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>

void initConnection(char *server,char *port, char* file, int fenstergroesse);
void sendRequest(struct request *paket);
struct answer* getAnswer();
void configSocket();
int exitSocket();
void printRequest(struct request *req);
void printAnswer(struct answer *ans);