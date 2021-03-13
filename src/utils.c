#include "colormyday.h"

/*
 * STRING TOOLS
 */
char** split_args(char* string) {
	int len = 0;
	char** ret = malloc(sizeof(char*));

	char* chunk = NULL;
	char* token;
	char* temp;
	bool quotes = false;
	while ((token = strsep(&string, " "))) {
		if (!quotes) {
			if ((token[0] == '"') && (token[strlen(token)-1] == '"')) {
				token++;
				token[strlen(token)-1] = 0;
				chunk = token;

			} else if (token[0] == '"') {
				quotes = true;
				token++;
				chunk = token;

			}

		} else {
			if (token[strlen(token)-1] == '"') {
				quotes = false;
				token[strlen(token)-1] = 0;
				
				asprintf(&temp, "%s %s", chunk, token);
				strcpy(chunk, temp);
				free(temp);

				token = chunk;

			} else {
				asprintf(&temp, "%s %s", chunk, token);
				strcpy(chunk, temp);
				free(temp);

				continue;

			}
		}

		ret = realloc(ret, sizeof(char*) * (len + 2));
		ret[len] = token;
		len = len + 1;

	}

	ret = realloc(ret, sizeof(char*) * (len + 2));
	ret[len] = 0;
	return ret;

}

/*
 * DICTIONARY TOOLS
 */

/*
 * Function: stringstring_dict
 *
 * In:
 *  list: the linked list "dictionary"
 *  c: the key string to be found.
 *
 * Out:
 *  The value string corresponding to the key.
 *
 */
char* 
stringstring_dict(
	struct stringstring_llist* list, 
	char* c
) {
	char* ret = NULL;
	struct stringstring_llist* temp = list;	

	while(temp) {
		if (strcmp(temp->content_1, c) == 0) {
			ret = temp->content_2;
			break;
		}
		temp = temp->next;
	}

	return ret;
}

/*
 * Function: stringint_dict
 *
 * In:
 *  list: the linked list "dictionary"
 *  c: the key string to be found.
 *
 * Out:
 *  The value int corresponding to the key.
 *
 */
int 
stringint_dict(
	struct stringint_llist* list, 
	char* c
) {
	int ret = 0;
	struct stringint_llist* temp = list;

	while (temp) {
		if (strcmp(temp->c_content, c) == 0) {
			ret = temp->i_content;
			break;
		}
		temp = temp->next;
	}
	
	return ret;
}

/*
 * LINKED LIST TOOLS PUSH
 */

void push_stringstring_llist(char* c_1, char* c_2, struct stringstring_llist** list) {
	struct stringstring_llist* ret;
	ret = malloc(sizeof(struct stringstring_llist));
	ret->content_1 = c_1;
	ret->content_2 = c_2;
	ret->next = *list;
	
	*list = ret;
}

/*
 * Function: push_stringint_llist
 *
 * In:
 *  c_content: a string (char(acter)p(ointer))
 *  i_content: an integer (int(eger))
 *  list: a pointer to a pointer to a string, int struct linked list
 *
 * This function prepends a stringint llist containing the given data
 * to the given stringint llist.
 *
 */
void 
push_stringint_llist(
	char* c_content, 
	int i_content, 
	struct stringint_llist** list
) {
	struct stringint_llist* ret;
	ret = malloc(sizeof(struct stringint_llist));
	ret->c_content = c_content;
	ret->i_content = i_content;
	ret->next = *list;

	*list = ret;
}

void push_string_llist(char* name, struct string_llist** list) {
	struct string_llist* ret;
	ret = malloc(sizeof(struct string_llist));
	ret->content = name;
	ret->next = *list;

	*list = ret;
}

void push_event_llist(struct event event, struct event_llist** list) {
	struct event_llist* ret;
	ret = malloc(sizeof(struct event_llist));
	ret->event = event;
	ret->next = *list;
	
	*list = ret;
}

/*
 * Function: push_display_event_llist
 *
 * In:
 *  display_event: a display event
 *  list: a pointer to a pointer to a linked list of event pointers
 *
 * This function prepends a display event llist containing the given
 * display event to the given display event llist.
 *
 * Display_event_llists made like this will need to be freed.
 *
 */
void 
push_display_event_llist(
	struct display_event display_event, 
	struct display_event_llist** list
) {
	struct display_event_llist* ret;
	ret = malloc(sizeof(struct display_event_llist));
	ret->display_event = display_event;
	ret->next = *list;

	*list = ret;

}

/*
 * LINKED LIST TOOLS FREE
 */

void 
free_stringstring_llist(
	struct stringstring_llist* list,
	bool malloced_1,
	bool malloced_2
) {
	struct stringstring_llist* temp = list;
	struct stringstring_llist* tempp;

	while (temp) {
		if (malloced_1) {
			free(temp->content_1);

		}

		if (malloced_2) {
			free(temp->content_2);

		}

		tempp = temp;
		temp = temp->next;

		free(tempp);

	}
}

void 
free_stringint_llist(
	struct stringint_llist* list,
	bool malloced
) {
	struct stringint_llist* temp = list;
	struct stringint_llist* tempp;

	while (temp) {
		if (malloced) {
			free(temp->c_content);

		}

		tempp = temp;
		temp = temp->next;

		free(tempp);

	}
}

/* char*s in this llist are either all malloc'ed or not */
void free_string_llist(struct string_llist* list, bool malloced) {
	struct string_llist* temp = list;
	struct string_llist* tempp;
	
	while (temp) {
		if (malloced) {
			free(temp->content);

		}

		tempp = temp;
		temp = temp->next;

		free(tempp);
	
	}
}

/* void free_event_llist(struct event_llist* list) { */


/* } */

void 
free_display_event_llist(
	struct display_event_llist* list
) {
	struct display_event_llist* temp = list;
	struct display_event_llist* tempp;

	while (temp) {
		tempp = temp;
		temp = temp->next;

		free(tempp->display_event.event.name);
		free(tempp->display_event.event.note);
		free(tempp);

	}
}

/*
 * LINKED LIST TOOLS DUMP
 */
void dump_stringstring_llist(struct stringstring_llist* list) {
	struct stringstring_llist* temp = list;

	while(temp) {
		temp = temp->next;
	}
}

void dump_stringint_llist(struct stringint_llist* list) {
	struct stringint_llist* temp = list;

	while(temp) {
		printf("C: %s, I: %d\n", temp->c_content, temp->i_content);
		temp=temp->next;
	}
}

void dump_string_llist(struct string_llist* list) {
	struct string_llist* temp = list;
	while (temp) {
		printf("%s\n", temp->content);
		temp = temp->next;
	}
}

/* Event dumper */
void dump_event(struct event event) {
	int start_time = event.start_time;
	int end_time = event.end_time;
	char* name = event.name;
	printf("EVENT: %s, %d-%d\n", name, start_time, end_time);
}

void dump_display_event(struct display_event display_event) {
	int start_time = display_event.event.start_time;
	int end_time = display_event.event.end_time;
	char* name = display_event.event.name;
	char* group = display_event.group;
	int color = display_event.color;
	printf("EVENT: %s, %s, %d-%d, %d\n", name, group, start_time, end_time, color);
}

void dump_event_llist(struct event_llist* list) {
	struct event_llist* temp = list;
	while (temp) {
		dump_event(temp->event);
		temp = temp->next;

	}
}

/* Misc. dumper */
void dump_cursor(int y, int x) {
	printf("y: %d, x: %d\n", y, x);

}

void print_cursor(int y, int x) {
	mvwprintw(rainbow, y+3, x+8, "C");
	wrefresh(rainbow);
}

void dump_stringp(char** stringp) {
	int i = 0;
	while (stringp[i]) {
		printf("%s\n", stringp[i]);
		i += 1;

	}
}

/*
 * TIME TOOLS
 */
/* struct tm* time_to_tm_local(int time) { */
/* 	time_t temp = time; */
/* 	struct tm* temp_tm = localtime(&temp); */
/* 	struct tm* ret = malloc(sizeof(struct tm)); */

/* 	*ret = *temp_tm; */

/* 	return ret; */

/* } */

/*
 * Function: tm_add_interval
 *
 * In:
 *  tm: the base tm
 *  years: the number of years to be added
 *  months: the number of months to be added
 *  days: the number of days to be added
 *
 * Out:
 *  The resulting, normalized tm.
 *
 * This function adds (moves forward in time by) certain quantities of
 * years, months, and days to a tm struct. It then normalizes it.
 *
 */
struct tm 
tm_add_interval(
	struct tm tm, 
	int years, 
	int months, 
	int days
) {
	struct tm temp = tm;

	temp.tm_year = temp.tm_year + years;
	temp.tm_mon = temp.tm_mon + months;
	temp.tm_mday = temp.tm_mday + days;
	temp.tm_isdst = -1;

	time_t temp_t = mktime(&temp);

	return *localtime(&temp_t);

}

int string_to_time(char* s) {
	regex_t stamp_reg, duration_reg;
	regcomp(&stamp_reg, "[0-9]*-[0-9]*-[0-9]*-[0-9]*-[0-9]*", 0);
	regcomp(&duration_reg, "[0-9]*h|[0-9]*m|[0-9]*h[0-9]*m", 0);

	int stamp_reg_match = regexec(&stamp_reg, s, 0, NULL, 0);
	int duration_reg_match = regexec(&duration_reg, s, 0, NULL, 0);

	regfree(&stamp_reg);
	regfree(&duration_reg);

	if (stamp_reg_match && duration_reg_match) {
		return(-1);

	}

	char* temp_s = strdup(s);

	int data[5];
	
	char* token = strtok(temp_s, "-");

	int i = 0;
	while (token != NULL) {
		data[i] = atoi(token);
		token = strtok(NULL, "-");

		i = i + 1;

	}

	struct tm time;
	time.tm_year = data[0] - 1900;
	time.tm_mon = data[1] - 1;
	time.tm_mday = data[2];
	time.tm_hour = data[3];
	time.tm_min = data[4];
	time.tm_sec = 0;
	time.tm_isdst = -1;

	free(temp_s);

	return (int)mktime(&time);

}

struct tm end_of_day(struct tm tm) {
	struct tm temp = tm;

	temp.tm_hour = 23;
	temp.tm_min = 59;
	temp.tm_sec = 59;

	return temp;

}

struct tm start_of_day(struct tm tm) {
	struct tm temp = tm;

	temp.tm_hour = 0;
	temp.tm_min = 0;
	temp.tm_sec = 0;

	return temp;

}

char* event_duration(int start_time, int end_time) {
		char* ret;

		int duration = end_time - start_time;
		int h = duration / 3600;
		int m = (duration % 3600)/60;
		int s = (duration % 3600) % 60;
	
		asprintf(&ret, "%d:%d:%d", h, m, s);

		return ret;
}
