#include "colormyday.h"
#include "gui.h"

struct tm                  earlier_bound_day;
struct tm                  later_bound_day;
struct dsp_eventp_llist*   current_events = NULL;
struct dsp_event*          current_event = NULL;
struct stringstring_llist* member_group_dict = NULL;
struct stringstring_llist* group_hex_dict = NULL;
struct stringint_llist*    group_color_dict = NULL;
int                        current_time;

/**************************************************************************************
* REGARDING COLOR GLOBALS *************************************************************
***************************************************************************************/
static
int 
make_color(
	char* code,
	struct error** err
) {
	/* Validate hex color code */
	int good_hex = regex_match("^[0-9a-fA-F]{6}$", code);
	if (!good_hex) {
		init_error(err, BAD_HEX_COLOR_CODE, FATAL, "%s", code);
		return 0;
	}

	/* Create new color pair */
	char red[3];
	char green[3];
	char blue[3];
	static int no_of_colors = 50; // why 50?
	no_of_colors = no_of_colors + 2;
	strncpy(red, code, sizeof(red));
	red[sizeof(red)-1] = 0; // assigning?
	code = code + 2;
	strncpy(green, code, sizeof(green));
	green[sizeof(green)-1] = 0;
	code = code + 2;
	strncpy(blue, code, sizeof(blue));
	blue[sizeof(blue)-1] = 0;
	char* redhex;
	char* greenhex;
	char* bluehex;
	asprintf(&redhex, "0x%s", red);
	asprintf(&greenhex, "0x%s", green);
	asprintf(&bluehex, "0x%s", blue);
	int r, g, b;
	r = round(((int)strtol(redhex, NULL, 0)/(float) 255)*1000);
	g = round(((int)strtol(greenhex, NULL, 0)/(float) 255)*1000);
	b = round(((int)strtol(bluehex, NULL, 0)/(float) 255)*1000);
	free(redhex);
	free(greenhex);
	free(bluehex);
	init_color(no_of_colors, r, g, b);
	float luma = 0.2126*(r/(float) 1000) + 
		0.7152*(g/(float) 1000) + 
		0.0722*(b/(float) 1000);
	if (luma > 0.4) {
		init_pair(no_of_colors, 10, no_of_colors);
	} else {
		init_pair(no_of_colors, 11, no_of_colors);
	}
	return no_of_colors;
}

static
void 
make_group_color_dict( // make this functional
	struct stringint_llist** list,
	struct error** err
) {
	struct stringstring_llist* temp = group_hex_dict;
	*list = NULL;
	while (temp) {
		int new_color = make_color(temp->content_2, err);
		if ERRP return;
		push_stringint_llist(temp->content_1, new_color, list);
		temp = temp->next;
	}
}

/**************************************************************************************
* REGARDING EVENT GLOBALS *************************************************************
***************************************************************************************/
static
struct dsp_event*
categorize(
	struct event* event
) {
	struct dsp_event* dsp_event = malloc_dsp_event();
	dsp_event->event = event;
	int color = -1; // maybe choose a different default...
	char* group = stringstring_dict(member_group_dict, event->name);
	if (group) {
		color = stringint_dict(group_color_dict, group);
	}
	dsp_event->group = group;
	dsp_event->color = color;
	return dsp_event;
}

/* Lists events from youngest (current_event) to oldest */
static 
struct dsp_eventp_llist* 
events_to_dsp_events(
	struct string_llist* list
) {
	struct dsp_eventp_llist* current_events_temp = NULL;
	struct string_llist* temp = list;
	struct dsp_event* temp_dsp_event;
	struct event* file_event;
	while (temp) { // do error handling
		file_event = file_to_event(temp->content);
		temp_dsp_event = categorize(file_event);
		push_dsp_eventp_llist(temp_dsp_event, &current_events_temp);
		temp = temp->next;
	}
	return current_events_temp;
}

void 
reload_current_events(
	struct error** err
) {
	/* Free existing event data globals */
	free_dsp_eventp_llist(current_events);
	current_event = NULL;

	/* Remake current_events global */
	struct string_llist* events = get_events_between(earlier_bound_day, later_bound_day);
	current_events = events_to_dsp_events(events);
	free_string_llist(events, true);

	/* Remake current_event global */
	struct event* last = last_event(err);
	if ERRP return;
	if (last == NULL) {
		return;
	}
	if (last->end_time == TILL_NOW) {
		current_event = categorize(last);
	}
}

/****************************************************************************
* EVENT ACTIONS *************************************************************
*****************************************************************************/
void
end_current_event_gui(
	int late_time,
	struct error** err
) {
	end_current_event_core(late_time, err);
	if ERRP return;
	reload_current_events(err);
	if ERRP return;
}

void
begin_event_gui(
	char* name,
	int late_time,
	struct error** err
) {
	begin_event_core(name, late_time, err);
	if ERRP return;
	reload_current_events(err);
	if ERRP return;
}

/*********************************************************************
 * INITALIZER ********************************************************
 *********************************************************************/
void 
gui_data_init(
	int rainbow_h,
	struct error** err
) {
	/* Make time globals */
	current_time = time(NULL);
	int days_back = rainbow_h - 5;
	time_t temp = current_time;
	struct tm* current_tm = malloc(sizeof(struct tm));
	localtime_r(&temp, current_tm);
	earlier_bound_day = start_of_day(tm_add_interval(
		*current_tm, 0, 0, -days_back));
	localtime_r(&temp, current_tm);
	later_bound_day = end_of_day(tm_add_interval(
		*current_tm, 0, 0, 0));
	free(current_tm);

	/* Load color globals */
	make_member_group_hex_dicts(err);
	if ERRP return;
	make_group_color_dict(&group_color_dict, err);
	if ERRP return;

	/* Load event globals */
	reload_current_events(err);
	if ERRP return;
}



