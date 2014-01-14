#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>
#include <time.h>

void initConnection(char *empfIP,char *port, char* file, int fenstergroesse);
void sendRequest(struct request *paket);
struct answer* getAnswer();
int exitSocket();
void printRequest(struct request *req);
void printAnswer(struct answer *ans);
int antwort_erhalten(clock_t timer);