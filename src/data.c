#include "colormyday.h"

/* the current time in epoch seconds */
int current_time;

/* a tm corresponding to the first second of the first of the visible
 * days */
struct tm earlier_bound_day;

/* a tm corresponding to the last second of the last of the visible
 * days */
struct tm later_bound_day;

/* a linked list of pointers to display events corresponding to all
 * currently visible events */
struct display_event_llist* current_events;

/* a display event */
struct display_event current_event;

struct stringstring_llist* member_group_dict = NULL;
struct stringstring_llist* group_hex_dict = NULL;
struct stringint_llist* group_color_dict = NULL;

/* placeholder event */
struct event null_event;

/* placeholder display event */
struct display_event null_display_event;

/*
 * FREE
 */
void 
free_data(
	void
) {	
	free_display_event_llist(current_events);
	free_stringstring_llist(member_group_dict, false, false);
	free_stringstring_llist(group_hex_dict, false, false);
	free_stringint_llist(group_color_dict, false);

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
 * of the color in question. Foreground is dark if the color is light, 
 * and vice versa.
 *
 */
static
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
static
void 
make_group_color_dict(
	struct stringint_llist** list
) {
	struct stringstring_llist* temp = group_hex_dict;
	*list = NULL;

	while (temp) {
		push_stringint_llist(
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
 * Function: last_event
 *
 * Out:
 *  The event struct corresponding to last recorded event.
 *
 * This function gets the parameters of the most recently begun
 * event.
 *
 */
struct event
last_event(
	void
) {
	char* path = last_event_path();

	if (path != NULL) {
		return file_to_event(path);

	}

	return null_event;

}

/*
 * Function: begin_event
 *
 * In:
 *  name: a string containing the name of the new event
 *  late_time: the epoch timestamp containing the time at which the 
 *  new event should be retroactively begun, or 0 (if it should not
 *  be retroactively begun)
 *
 * This function creates a new event using the given data, prints it
 * to a file, and then sets current_event to it.
 *
 */
void
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

	char* malloc_note = malloc(sizeof(char)*3);
	strcpy(malloc_note, "-1");
	struct event temp = (struct event) {
		.start_time = temp_time,
		.end_time = -1,
		.name = name,
		.note = malloc_note
	};

	event_to_file(temp);

	reload_current_events();

}

/*
 * Function: end_current_event
 *
 * In:
 *  late_time: an integer representing the unix timestamp at which 
 *  the current event should be retroactively ended, or 0
 *
 * This function ends current_event at the given time. 
 *
 */
void
end_current_event(
	int late_time
) {
	int temp_time;
	if (late_time != 0) {
		temp_time = late_time;

	} else {
		temp_time = current_time;

	}

	current_event.event.end_time = temp_time;
	event_to_file(current_event.event);

	reload_current_events();

}

/*
 * Function: categorize
 *
 * In:
 *  event: an event struct
 *
 * Out:
 *  a display event with group and color set according to the
 *  globals
 *
 * This function converts an event into a display-ready display
 * event
 *
 */
static
struct display_event
categorize(
	struct event event
) {
	struct display_event display_event;

	display_event.event = event;

	int c = 1;
	char* group = NULL;
	group = stringstring_dict(member_group_dict, event.name);
	if (group) {
		c = stringint_dict(group_color_dict, group);

	}

	display_event.group = group;
	display_event.color = c;

	return display_event;

}

/* 
 * Function: make_current_events
 *
 * In:
 *  list: a linked list of paths to event files
 *
 * Out:
 *  A linked list of display events corresponding to the given paths
 *  and filled with the correct groups and colors as given by the
 *  dicts.
 *
 * This function is to be used on lists of event paths corresponding
 * to a period of time as given by get_events_between. mg_dict is to
 * be the global member_group_dict and gc_dict is to be the global
 * group_color_dict.
 *
 * The order given by the input list is preserved.
 *
 */
/* Lists events from youngest (current_event) to oldest */
static 
struct display_event_llist* 
make_current_events(
	struct string_llist* list
) {
	struct string_llist* temp = list;
	struct display_event_llist* current_events_temp = NULL;

	struct event temp_event;
	struct display_event temp_display_event;

	while (temp) {
		temp_event = file_to_event(temp->content);

		temp_display_event = categorize(temp_event);
		
		push_display_event_llist(temp_display_event, &current_events_temp);
		temp = temp->next;

	}

	return current_events_temp;

}

/*
 * Function: reload_current_events
 *
 * This function recreates current_events with the current global
 * bounds by re-reading and processing local files. It also remakes
 * the current_event global.
 *
 */
void 
reload_current_events(
	void
) {
	free_display_event_llist(current_events);

	/* get event files and make them into current_events */
	struct string_llist* events = get_events_between(earlier_bound_day, later_bound_day);
	current_events = make_current_events(events);

	/* get the last event and make it the current, if it is */
	struct event last = last_event();
	if (last.end_time == -1) {
		current_event = categorize(last);

	} else {
		current_event = null_display_event;

	}

	free_string_llist(events, true);

}

/*
 * INITALIZER
 */
/*
 * Function: data_init
 *
 * In:
 *  rainbow_h: the height of the rainbow
 *
 * This function initializes all 
 *
 */
void 
data_init(
	int rainbow_h
) {
	current_time = time(NULL);

	if (rainbow_h == -1) {
		time_t temp = 0;

		localtime_r(&temp, &earlier_bound_day);
		localtime_r(&temp, &later_bound_day);

	} else {
		/* Initialize rainbow to display to today from (rainbow_h - 3)/2 days ago */
		int earlier_bound_relative_days = (rainbow_h - 1) - 4;
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

	/* create null globals */
	null_event = (struct event) {
		.name = NULL,
		.note = NULL,
		.start_time = -1,
		.end_time = -1
	
	};
	
	null_display_event = (struct display_event) {
		.event = null_event,
		.group = NULL,
		.color = 0
	
	};

	/* Load event globals */
	reload_current_events();

}



