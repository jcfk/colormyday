#include "colormyday.h"

int current_time;
struct tm earlier_bound_day;
struct tm later_bound_day;
struct display_eventp_llist* current_events;
struct display_event current_event;

struct charpcharp_llist* member_group_dict = NULL;
struct charpcharp_llist* group_hex_dict = NULL;
struct charpint_llist* group_color_dict = NULL;

/*
 * FREE
 */

void 
free_data(
	void
) {	
	free_display_eventp_llist(current_events);
	free_charpcharp_llist(member_group_dict, false, false);
	free_charpcharp_llist(group_hex_dict, false, false);
	free_charpint_llist(group_color_dict, false);

}

/*
 * COLOR DATA
 */

/*
 * Function: make_color
 *
 * In:
 *  code: a string containing a hex code (ex: "dd4e13")
 *
 * Out:
 *  Integer representing a new color pair.
 *
 * Turns a hex color string into a curses color pair with a background
 * of the color in question. Foreground is dark if bg is light, and
 * vice versa.
 *
 */
int 
make_color(
	char* code
) {
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

	free(redhex);
	free(greenhex);
	free(bluehex);

	return no_of_colors;

}

/*
 * Function: make_group_color_dict
 *
 * In:
 *  list: a pointer to a linked list which will be filled with 
 *  event groups and their associated colors
 *
 * Utilizes global data associating event groups with hex values
 * to fill a given linked list associating event groups with
 * corresponding curses color pairs.
 *
 */
void 
make_group_color_dict(
	struct charpint_llist** list
) {
	struct charpcharp_llist* temp = group_hex_dict;
	*list = NULL;

	while (temp) {
		push_charpint_llist(
			temp->content_1,
			make_color(temp->content_2),
			list
		);
		temp = temp->next;

	}
}

/*
 * EVENT DATA
 */

/*
 * Function: time_to_event
 *
 * In:
 *  time: unix epoch time
 *
 * Out:
 *  the display event which contains the time in question, or if 
 *  none exists, a null display event
 *
 * Finds a display event corresponding to an event containing a given time.
 *
 */
struct display_event 
time_to_event(
	int time
) {
	struct display_eventp_llist* temp = current_events;

	int start_time, end_time;
	while (temp) {
		start_time = temp->display_event.event.start_time;
		end_time = temp->display_event.event.end_time;

		if ((start_time < time) 
			&& ((time <= end_time) 
				|| ((end_time == -1)
					&& (time <= current_time)))) {
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
	ret.color = 0;

	return ret;

}

/*
 * Function: begin_event
 *
 * In:
 *  name: a string containing the name of the new event
 *  late_time: a timestamp containing the time at which the new
 *  event should be retroactively begun, or 0
 *
 * Out:
 *  a pointer to the last visible display event, which will be the
 *  current event if it is visible.
 *
 * This function creates a new event using the given data and sets
 * current_event to it. 
 *
 * If it should be visible, it is appended to display_events.
 * disp_event is called on the last visible event (which will be the
 * current event if it is visible) in order to intialize its display
 * position parameters.
 *
 */
struct display_event*
begin_event(
	char* name, 
	int late_time
) {
	int temp_time;
	if (late_time != 0) {
		temp_time = late_time;

	} else {
		temp_time = current_time;

	}

	char* malloc_note = malloc(sizeof(char) * 3);
	strcpy(malloc_note, "-1");
	struct event temp = (struct event) {
		.start_time = temp_time,
		.end_time = -1,
		.name = name,
		.note = malloc_note
	};

	event_to_file(temp);

	make_current_event(temp);

	if (mktime(&earlier_bound_day) <= temp_time
		&& temp_time <= mktime(&later_bound_day)) {
		push_display_eventp_llist(current_event, &current_events);

	}

	return &(current_events->display_event);

}

/*
 * Function: end_current_event
 *
 * In:
 *  late_time: an integer representing the unix timestamp at which 
 *  the current event should be retroactively ended, or 0
 *
 * Out:
 *  a pointer to the display event of the last visible event. If 
 *  the current event is visible this will be the current event's 
 *  display event.
 *
 * This function ends current_event at the given time. 
 *
 * If the current event is visible, the last visible event is 
 * modified as well. disp_event is called on the last visible event
 * in order to modify its display position parameters.
 *
 */
struct display_event* 
end_current_event(
	int late_time
) {
	int temp_time;
	if (late_time != 0) {
		temp_time = late_time;

	} else {
		temp_time = current_time;

	}

	if (current_events->display_event.event.start_time == current_event.event.start_time) {
		current_events->display_event.event.end_time = temp_time;

	}

	current_event.event.end_time = temp_time;
	event_to_file(current_event.event);

	current_event.event.name = NULL;

	disp_event(&(current_events->display_event), false);

	return &(current_events->display_event);

}

/* 
 * Function: make_current_events
 *
 * In:
 *  list: a list of 
 *
 */
/* Lists events from youngest (current_event) to oldest */
static struct display_eventp_llist* 
make_current_events(
	struct charp_llist* list,
	struct charpcharp_llist* mg_dict,
	struct charpint_llist* gc_dict
) {

	struct charp_llist* temp = list;
	struct display_eventp_llist* current_events_temp = NULL;

	struct event temp_event;
	struct display_event temp_display_event;

	while (temp) {
		temp_event = file_to_event(temp->content);
		temp_display_event.event = temp_event;

		int c = 1;
		char* group = NULL;
		group = charpcharp_dict(mg_dict, temp_event.name);
		if (group) {
			c = charpint_dict(gc_dict, group);

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

/*
 * Function: make_current_event
 *
 * In:
 *  event: an event containing data to be made into the current event
 *
 * This function initializes a new current_event global using given
 * data.
 *
 */
void 
make_current_event(
	struct event event
) {
	int c = 1;

	char* group;
	group = charpcharp_dict(member_group_dict, event.name);
	if (group) {
		c = charpint_dict(group_color_dict, group);

	}

	struct display_event temp_de;
	temp_de.event = (struct event) {
		.start_time = event.start_time,
		.end_time = event.end_time,
		.name = event.name,
		.note = event.note

	};

	temp_de.group = group;
	temp_de.color = c;

	current_event = temp_de;

}

/*
 * Function: reload_current_events
 *
 * This function 
 *
 */
void 
reload_current_events(
) {
	free_display_eventp_llist(current_events);

	struct charp_llist* events = get_events_between(earlier_bound_day, later_bound_day);
	current_events = make_current_events(events, member_group_dict, group_color_dict);

	free_charp_llist(events, true);

}

void scroll_current_events(enum rainbow_scroll direction) {
	struct display_eventp_llist* new_events;

	if (direction == R_UP) {
		/* add new events */
		struct charp_llist* temp_1 = get_events_between(earlier_bound_day, tm_add_interval(earlier_bound_day, 0, 0, 1) );
		new_events = make_current_events(temp_1, member_group_dict, group_color_dict);

		free_charp_llist(temp_1, true);

		if (new_events != NULL) { 
			struct display_eventp_llist* temp;
			struct tm temp_tm = tm_add_interval(earlier_bound_day, 0, 0, -1);
			if (new_events->display_event.event.end_time >= mktime(&temp_tm)) {
				temp = new_events;
				new_events = new_events->next;

				free(temp->display_event.event.name);
				free(temp->display_event.event.note);
				free(temp);

			}

			struct display_eventp_llist* temp_3 = current_events;
			while (temp_3->next) {
				temp_3 = temp_3->next;

			}

			temp_3->next = new_events;

		}

		/* remove old events */
		struct display_eventp_llist* temp_2;
		while (current_events->display_event.event.start_time >= mktime(&later_bound_day)) {
			temp_2 = current_events;
			current_events = current_events->next;

			free(temp_2);
			temp_2 = NULL;

		}
	} else {
		/* add new events */
		struct charp_llist* temp_1 = get_events_between(tm_add_interval(later_bound_day, 0, 0, -1), later_bound_day);
		new_events = make_current_events(temp_1, member_group_dict, group_color_dict);

		free_charp_llist(temp_1, true);

		if (new_events != NULL) {
			struct display_eventp_llist* temp_2 = new_events;
			struct display_eventp_llist* temp_3 = temp_2;

			while (temp_2->next) {
				temp_3 = temp_2;
				temp_2 = temp_2->next;

			}

			struct tm temp_tm = tm_add_interval(later_bound_day, 0, 0, -1);
			if (temp_2->display_event.event.start_time <= mktime(&temp_tm)) {
				temp_3->next = current_events;
				free(temp_2->display_event.event.name);
				free(temp_2->display_event.event.note);
				free(temp_2);

			} else {
				temp_2->next = current_events;

			}
			
			current_events = new_events;

		}

		/* remove old events */
		struct display_eventp_llist* temp_4 = current_events;
		while (((temp_4->display_event.event.end_time >= mktime(&earlier_bound_day)) || (temp_4->display_event.event.end_time == -1)) && (temp_4->next != NULL)) {
			temp_4 = temp_4->next;

		}

		free_display_eventp_llist(temp_4->next);
		temp_4->next = NULL;

	}
}

/*
 * INITALIZER
 */
void data_init(int rainbow_h) {
	current_time = time(NULL);

	if (rainbow_h == -1) {
		time_t temp = 0;

		localtime_r(&temp, &earlier_bound_day);
		localtime_r(&temp, &later_bound_day);

	} else {
		/* Initialize rainbow to display to today from (rainbow_h - 3)/2 days ago */
		int earlier_bound_relative_days = (rainbow_h - 3) - 4;
		int later_bound_relative_days = 0;

		time_t temp = current_time;

		struct tm* current_tm = malloc(sizeof(struct tm));
		localtime_r(&temp, current_tm);
		earlier_bound_day = start_of_day(tm_add_interval(*current_tm, 0, 0, -1*earlier_bound_relative_days));

		localtime_r(&temp, current_tm);
		later_bound_day = end_of_day(tm_add_interval(*current_tm, 0, 0, -1*later_bound_relative_days));

		free(current_tm);

	}

	/* Load color globals */
	make_member_group_hex_dicts(&member_group_dict, &group_hex_dict);
	make_group_color_dict(&group_color_dict);

	/* Load event globals */
	reload_current_events();

}
