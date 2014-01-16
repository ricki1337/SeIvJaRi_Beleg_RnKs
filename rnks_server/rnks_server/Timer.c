#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "Timer.h"

//#define DEBUG

struct timeouts* add_timer(struct timeouts *list, int timer_val, unsigned long seq_nr){
	struct timeouts *help, *new_elem;
	int sum = 0;
	
	new_elem = (struct timeouts*)malloc( sizeof (struct timeouts));
	if (list == NULL){
		list = new_elem;
		list->next = NULL;
	}else{
		//gehe ans ende
		help = list;
		while(help->next != NULL) help = help->next;
		//füge neues elem hinzu
		help->next = new_elem;
		//fülle elem mit daten
		help->next->next = NULL;
		/*if (timer_val < list->timer){ //must be the first element
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
		}*/
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
		list = list->next;
	}else{
		help = helper->next;
		while (help != NULL) {
			if (help->seq_nr == seq_nr){
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

void decrement_timer( struct timeouts *list){
	struct timeouts *help;
	if (list == NULL) return;
		help=list;
		#ifdef DEBUG
		if (list==NULL) printf( "del_timer: LIST empty \n");
		#endif
		while (help != NULL){
			if(help->timer > 0) {
				help->timer--;
				#ifdef DEBUG
					printf( "decrement_timer:seq_nr %d \t timer%d \n", help->seq_nr,help->timer);
				#endif
			}
			help=help->next;
		}
	
	
	//if (list->timer) return 1;
	//else return 0; //indicating a timeout
}


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