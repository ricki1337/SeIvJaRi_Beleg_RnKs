#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "Timer.h"

int getTimeout(struct timeouts *list){
	struct timeouts *help;
	if (list == NULL) return -1; //es gibt keine timer
	help=list;
	while (help != NULL){
		if(help->timer == 0) return help->seq_nr;
		help=help->next;
	}
	return -1; //es gibt keine timeouts
}


struct timeouts* add_timer(struct timeouts *list, int timer_val, unsigned long seq_nr){
	struct timeouts *help, *new_elem;
	//int sum = e;
	
	new_elem = (struct timeouts*)malloc( sizeof (struct timeouts));
	if (list == NULL){
		list = new_elem;
		list->next = NULL;
	}else{
		if (timer_val < list->timer){ //must be the first element
			help = new_elem;
			list->timer = timer_val-timer_val; 	//this is now the second element,
												//must be relative to the previous one
			help->next = list;
			list = help;
		}else{
			help = list;
			timer_val = timer_val - help->timer;
			while(help->next != NULL){
				if ((timer_val - (help->next)->timer) >= 0){
					help= help->next;
					timer_val = timer_val - help->timer;
				}else break;
			}
			//new elementwill be inserted after help
			new_elem->next = help->next;
			help->next = new_elem;
		}
	}
	new_elem->seq_nr = seq_nr;
	new_elem->timer = timer_val;
	return list;
}

struct timeouts* del_timer( struct timeouts *list, unsigned long seq_nr){

	struct timeouts *help,*helper=list;
	if (list== NULL) return NULL;
	if (list->seq_nr == seq_nr){
		help = list;
		if(list->next != NULL) list->next->timer +=list->timer;
		list = list->next;
	}else{
		help = helper->next;
		while (help != NULL) {
			if (help->seq_nr == seq_nr){
				if(help->next != NULL) help->next->timer +=help->timer;
				helper->next = help->next;
				break;
			}else{
				helper = help;
				help = help->next;
			}
		}
	}
	if (help != NULL) free(help);
	return list;
}

int decrement_timer( struct timeouts *list){
	struct timeouts *help;
	if (list == NULL) return -1;
	list->timer--;

	#ifdef DEBUG
		//timer anzeigen
		showTimer(list);
	#endif
	
	if (list->timer) return -1;//orig 1
	else return list->seq_nr; //orig 0 - indicating a timeout
}

void showTimer(struct timeouts *list){
	struct timeouts *help=list;
	printf("Timerliste:\n");
	while(help != NULL){
		printf("tSq: %d\t%d\n",help->seq_nr,help->timer);
		help = help->next;
	}

}