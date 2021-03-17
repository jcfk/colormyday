#include "colormyday.h"

/*
 * SCRIPT MODE
 */
/* void */ 
/* end_begin_script( */
/* 	char* name */
/* ) { */
/* 	if (current_event.event.name != NULL) { */
/* 		struct display_event last = *end_current_event(0); */
/* 		begin_event(name, 0); */
/* 		printf("Begun event: %s (ended %s)\n", name, last.event.name); */

/* 	} else { */
/* 		begin_event(name, 0); */
/* 		printf("Begun event: %s\n", name); */

/* 	} */

/* } */

/* void */ 
/* end_event_script( */
/* 	void */
/* ) { */
/* 	if (current_event.event.name != NULL) { */
/* 		struct display_event last = *end_current_event(0); */
/* 		char* t = event_duration(last.event.start_time, last.event.end_time); */
/* 		printf("Ended event: %s (duration %s)\n", last.event.name, t); */
		
/* 	} else { */
/* 		printf("No current event to end.\n"); */

/* 	} */
/* } */

/* void */ 
/* args_handle_script( */
/* 	int argc, */ 
/* 	char* argv[] */
/* ) { */
/* 	char* arg; */
/* 	int i = 1; */
/* 	while(i < argc) { */
/* 		arg = argv[i]; */

/* 		i += 1; */
/* 		if (strcmp(arg, "begin") == 0) { */
/* 			if (i == argc) { */
/* 				printf("ERR: Please enter the name of the event you'd like to begin. For example:\n\n\t$ colormyday begin Exercise\n\t$ colormyday begin \"Side Project\"\n\n"); */

/* 			} else { */
/* 				end_begin_script(argv[i]); */

/* 			} */

/* 		} else if (strcmp(arg, "end") == 0) { */
/* 			end_event_script(); */

/* 		} */
/* 	} */
/* } */


