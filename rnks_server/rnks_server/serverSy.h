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
void exitSocket();
int antwort_erhalten(clock_t timer);
void sendeReihenfolge(int *array,int size,int unordnung);