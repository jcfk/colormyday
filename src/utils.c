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
char* charpcharp_dict(struct charpcharp_llist* list, char* c) {
	char* ret = NULL;
	struct charpcharp_llist* temp = list;	

	while(temp) {
		if (strcmp(temp->content_1, c) == 0) {
			ret = temp->content_2;
			break;
		}
		temp = temp->next;
	}

	return ret;
}

int charpint_dict(struct charpint_llist* list, char* c) {
	int ret;
	struct charpint_llist* temp = list;

	while(temp) {
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
void push_charpcharp_llist(char* c_1, char* c_2, struct charpcharp_llist** list) {
	struct charpcharp_llist* ret;
	ret = malloc(sizeof(struct charpcharp_llist));
	ret->content_1 = c_1;
	ret->content_2 = c_2;
	ret->next = *list;
	
	*list = ret;
}

void push_charpint_llist(char* c_content, int i_content, struct charpint_llist** list) {
	struct charpint_llist* ret;
	ret = malloc(sizeof(struct charpint_llist));
	ret->c_content = c_content;
	ret->i_content = i_content;
	ret->next = *list;

	*list = ret;
}

void push_charp_llist(char* name, struct charp_llist** list) {
	struct charp_llist* ret;
	ret = malloc(sizeof(struct charp_llist));
	ret->content = name;
	ret->next = *list;

	*list = ret;
}

void push_eventp_llist(struct event event, struct eventp_llist** list) {
	struct eventp_llist* ret;
	ret = malloc(sizeof(struct eventp_llist));
	ret->event = event;
	ret->next = *list;
	
	*list = ret;
}

void push_display_eventp_llist(struct display_event display_event, struct display_eventp_llist** list) {
	struct display_eventp_llist* ret;
	ret = malloc(sizeof(struct display_eventp_llist));
	ret->display_event = display_event;
	ret->next = *list;

	*list = ret;
}

/*
 * LINKED LIST TOOLS FREE
 */
/* void free_charpcharp_llist(struct charpcharp_llist* list) { */
	

/* } */

/* void free_charpint_llist(struct charpint_llist* list) { */


/* } */

/* char*s in this llist are either all malloc'ed or not */
void free_charp_llist(struct charp_llist* list, bool malloced) {
	struct charp_llist* temp = list;
	struct charp_llist* tempp;
	
	while (temp) {
		if (malloced) {
			free(temp->content);

		}

		tempp = temp;
		temp = temp->next;

		free(tempp);
	
	}
}

/* void free_eventp_llist(struct eventp_llist* list) { */


/* } */

void free_display_eventp_llist(struct display_eventp_llist* list) {
	struct display_eventp_llist* temp = list;
	struct display_eventp_llist* tempp;

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
void dump_charpcharp_llist(struct charpcharp_llist* list) {
	struct charpcharp_llist* temp = list;

	while(temp) {
		temp = temp->next;
	}
}

void dump_charpint_llist(struct charpint_llist* list) {
	struct charpint_llist* temp = list;

	while(temp) {
		printf("C: %s, I: %d\n", temp->c_content, temp->i_content);
		temp=temp->next;
	}
}

void dump_charp_llist(struct charp_llist* list) {
	struct charp_llist* temp = list;
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

void dump_eventp_llist(struct eventp_llist* list) {
	struct eventp_llist* temp = list;
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

void dump_charpp(char** charpp) {
	int i = 0;
	while (charpp[i]) {
		printf("%s\n", charpp[i]);
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

struct tm tm_add_interval(struct tm tm, int years, int months, int days) {
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
