#include "colormyday.h"

int current_time;
int earlier_bound_day, later_bound_day;
struct display_eventp_llist* current_events;
struct display_event current_event;

struct charpcharp_llist* member_group_dict = NULL;
struct charpcharp_llist* group_hex_dict = NULL;
struct charpint_llist* group_color_dict = NULL;

/*
 * COLOR DATA
 */
int make_color(char* code) {
	static int no_of_colors = 50;

	char red[3];
	char green[3];
	char blue[3];
	
	no_of_colors = no_of_colors + 2;
	
	strncpy(red, code, sizeof(red));
	red[sizeof(red)-1] = 0;
	code = code + 2;

	strncpy(green, code, sizeof(green));
	green[sizeof(green)-1] = 0;
	code = code + 2;

	strncpy(blue, code, sizeof(blue));
	blue[sizeof(blue)-1] = 0;

	char* redhex;
	char* greenhex;
	char* bluehex;

	asprintf(&redhex, "%s%s", "0x", red);
	asprintf(&greenhex, "%s%s", "0x", green);
	asprintf(&bluehex, "%s%s", "0x", blue);
	
	int r, g, b;
	r = round(((int)strtol(redhex, NULL, 0)/(float) 255)*1000);
	g = round(((int)strtol(greenhex, NULL, 0)/(float) 255)*1000);
	b = round(((int)strtol(bluehex, NULL, 0)/(float) 255)*1000);

	init_color(no_of_colors, r, g, b);
	float luma = 0.2126*(r/(float) 1000) + 0.7152*(g/(float) 1000) + 0.0722*(b/(float) 1000);
	if (luma > 0.4) {
		init_pair(no_of_colors, 10, no_of_colors);
	} else {
		init_pair(no_of_colors, 11, no_of_colors);
	}

	return no_of_colors;
}

void make_group_color_dict(struct charpint_llist** list) {
	struct charpcharp_llist* temp = group_hex_dict;
	*list = NULL;

	while (temp) {
		push_charpint_llist(temp->content_1,
			make_color(temp->content_2),
			list);
		temp = temp->next;
	}
}

/*
 * EVENT DATA
 */
/* DEPRECATE IN FAVOR OF CURSOR TO EVENT */
struct display_event time_to_event(int time) {
	struct display_eventp_llist* temp = current_events;

	int start_time, end_time;
	while (temp) {
		start_time = temp->display_event.event.start_time;
		end_time = temp->display_event.event.end_time;

		if (start_time < time && (time < end_time || (end_time == -1 && time <= current_time))) {
			return temp->display_event;
		}

		temp = temp->next;
	}

	struct display_event ret;

	ret.event.start_time = 0;
	ret.event.end_time = 0;
	ret.event.name = NULL;
	ret.event.note = NULL;
	ret.group = NULL;
	ret.color = -1;

	return ret;
}

void begin_event(char* name) {
	struct event temp;
	temp.name = name;
	temp.note = "-1";
	temp.start_time = current_time;
	temp.end_time = -1;
	
	event_to_file(temp);
	
	make_current_event(temp);

	push_display_eventp_llist(current_event, &current_events);
}

struct display_event end_current_event() {
	if (current_event.event.name == NULL) {
		struct display_event temp;
		temp.event.name = NULL;

		return temp;
	}

	current_events->display_event.event.end_time = current_time;

	struct display_event temp = current_event;
	temp.event.end_time = current_time;
	current_event.event.name = NULL;
	
	event_to_file(temp.event);

	return temp;
	
}

/* Lists events from youngest (current_event) to oldest */
struct display_eventp_llist* make_current_events(struct charp_llist* list,
	struct charpcharp_llist* member_group_dict,
	struct charpint_llist* group_color_dict) 
	{

	struct charp_llist* temp = list;
	struct display_eventp_llist* current_events_temp = NULL;

	struct event temp_event;
	struct display_event temp_display_event;

	while(temp) {
		temp_event = file_to_event(temp->content);
		temp_display_event.event = temp_event;

		int c = 1;
		char* group = NULL;
		group = charpcharp_dict(member_group_dict, temp_event.name);
		if (group) {
			c = charpint_dict(group_color_dict, group);
		}

		temp_display_event.group = group;
		temp_display_event.color = c;

		if (temp_event.end_time == -1) {
			current_event = temp_display_event;
		}
		
		push_display_eventp_llist(temp_display_event, &current_events_temp);
		temp = temp->next;
	}

	return current_events_temp;
}

void make_current_event(struct event event) {
	struct display_event temp_de;
	
	int c = 1;
	char* group = NULL;
	group = charpcharp_dict(member_group_dict, event.name);
	if (group) {
		c = charpint_dict(group_color_dict, group);
	}

	temp_de.event = event;
	temp_de.group = group;
	temp_de.color = c;

	current_event = temp_de;
}

void reload_current_events() {
	current_events = make_current_events(get_events_between(earlier_bound_day, later_bound_day), member_group_dict, group_color_dict);
}

void scroll_current_events(enum rainbow_scroll direction) {
	struct display_eventp_llist* new_events;

	if (direction == R_UP) {
		/* add new events */
		new_events = make_current_events(get_events_between(earlier_bound_day, earlier_bound_day + 86400), member_group_dict, group_color_dict);

		if (new_events != NULL) { 
			if (new_events->display_event.event.end_time >= earlier_bound_day - 86400) {
				new_events = new_events->next;

			}

			struct display_eventp_llist* temp = current_events;
			while (temp->next) {
				temp = temp->next;

			}

			temp->next = new_events;

		}

		/* remove old events */
		while (current_events->display_event.event.start_time >= later_bound_day) {
			current_events = current_events->next;

		}

	} else {
		/* add new events */
		new_events = make_current_events(get_events_between(later_bound_day - 86400, later_bound_day), member_group_dict, group_color_dict);

		if (new_events != NULL) {
			struct display_eventp_llist* temp = new_events;
			struct display_eventp_llist* temp_2 = temp;

			while (temp->next) {
				temp_2 = temp;
				temp = temp->next;

			}

			if (temp->display_event.event.start_time <= later_bound_day - 86400) {
				temp_2->next = current_events;

			} else {
				temp->next = current_events;

			}

			current_events = new_events;

		}

		/* remove old events */
		struct display_eventp_llist* temp = current_events;
		while (((temp->display_event.event.end_time >= earlier_bound_day) || (temp->display_event.event.end_time == -1)) && (temp->next != NULL)) {
			temp = temp->next;

		}

		temp->next = NULL;

	}
}

/*
 * INITALIZER
 */
void data_init(int rainbow_h) {
	current_time = time(NULL);

	if (rainbow_h == -1) {
		earlier_bound_day = -1;
		later_bound_day = -1;

	} else {
		/* Initialize rainbow to display to today from (rainbow_h - 3)/2 days ago */
		int earlier_bound_relative_days = (rainbow_h - 3) - 4;
		int later_bound_relative_days = 0;
		
		struct tm earlier_tm = *time_to_tm_local(current_time -
			earlier_bound_relative_days*86400);
		struct tm later_tm = *time_to_tm_local(current_time -
			later_bound_relative_days*86400);

		earlier_bound_day = start_of_day(&earlier_tm);
		later_bound_day = end_of_day(&later_tm);

	}

	/* Load color globals */
	make_member_group_hex_dicts(&member_group_dict, &group_hex_dict);
	make_group_color_dict(&group_color_dict);

	/* Load event globals */
	reload_current_events();
}
