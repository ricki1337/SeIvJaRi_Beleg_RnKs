/* 	Data Oeclarations 
	used by C!
	Only Oata concerning Client and Server ! */
#define TIMEOUT_INT 200 //in Millisekunden
#define TIMEOUT 3 //must be a multiple of TIMEOUT_INT
#define MAX_WINDOW 10
#define DEFAULT_WINDOW 1
#define MAX_SEQNR 2*MAX_WINDOW-1
#define MAX_BUFFER 2*MAX_WINDOW

extern char *errorTable[];

struct request {
	unsigned char ReqType;
	#define ReqHello 'H' //ReqHello
	#define ReqData 'D' //ReqData
	#define ReqClose 'C' //ReqClose
	long FlNr; /*length of data in Byte
				if it is a Hello packet we transmit the window here*/
	unsigned long SeNr; //Sequence Number (== 0) beginn of file, if it is a Hello packet we can transmit fileArray size here
	#define PufferSize 256
	char name[PufferSize];
	char fname[50]; /* if it is a Hello packet we can transmit filename here */
};

struct answer {
	unsigned char AnswType;
	#define AnswHello 'H'
	#define AnswOk 'O'
	#define AnswWarn 'W'
	#define AnswClose 'C'
	#define AnswErr 0xFF
	unsigned FlNr;
	unsigned SeNo;
	#define ErrNo SeNo /* are identical*/
};