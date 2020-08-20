#include "colormyday.h"

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
 * LINKED LIST TOOLS DUMP
 */
void dump_charpcharp_llist(struct charpcharp_llist* list) {
	struct charpcharp_llist* temp = list;

	while(temp) {
		printf("1: %s, 2: %s\n", temp->content_1, temp->content_2);
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

/*
 * TIME TOOLS
 */
struct tm* time_to_tm_local(int time) {
	time_t temp = time;
	struct tm* ret = localtime(&temp);

	return ret;
}

int end_of_day(struct tm* tm) {
	int diff = 60*60*24 - (tm->tm_hour)*60*60 - (tm->tm_min)*60 - (tm->tm_sec);
	int time = mktime(tm);
	int temp = time + diff - 1;

	/* printf("End: %d\n", temp); */
	return temp;
}

int start_of_day(struct tm* tm) {
	int time = mktime(tm);
	int temp = time - (tm->tm_hour)*60*60 - (tm->tm_min)*60 - (tm->tm_sec);

	/* printf("Start: %d\n", temp); */
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
