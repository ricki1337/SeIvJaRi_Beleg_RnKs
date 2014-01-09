struct window {
	long SqNr;
	int ack;
	unsigned char AnswType;
	long error;
};

struct window* createWindowArray(unsigned int windows);

int getOpenWindows(struct window* windowArray);

int getNextFreeWindow(struct window* windowArray, unsigned long SqNr, unsigned char AnswType, long error);

int setWindowFree(struct window* windowArray, unsigned long SqNr);

int getWindowWithAck(struct window* windowArray);