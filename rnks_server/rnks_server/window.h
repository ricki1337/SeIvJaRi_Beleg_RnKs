struct window {
	long SqNr;
	int ack;
};

struct window* createWindowArray(unsigned int windows);

int getOpenWindows(struct window* windowArray);

int getNextFreeWindow(struct window* windowArray, unsigned long SqNr);

int setWindowFree(struct window* windowArray, unsigned long SqNr);