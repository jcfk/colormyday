#include "colormyday.h"

struct event time_to_event(int time) {


}

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

	/* printf("N:%d R:%d G:%d B:%d\n", no_of_colors, r, g, b); */

	init_color(no_of_colors, r, g, b);
	float luma = 0.2126*(r/(float) 1000) + 0.7152*(g/(float) 1000) + 0.0722*(b/(float) 1000);
	if (luma > 0.4) {
		init_pair(no_of_colors, 10, no_of_colors);
	} else {
		init_pair(no_of_colors, 11, no_of_colors);
	}

	return no_of_colors;
}

void make_group_color_dict() {
	struct charpcharp_llist* temp = group_hex_dict;
	group_color_dict = NULL;

	while (temp) {
		push_charpint_llist(temp->content_1,
			make_color(temp->content_2),
			&group_color_dict);
		temp = temp->next;
	}
}

void make_current_events(struct charp_llist* list) {
	struct charp_llist* temp = list;
	current_events = NULL;

	struct event temp_event;
	while(temp) {
		temp_event = file_to_event(temp->content);

		if (temp_event.end_time == -1) {
			current_event = temp_event;
		}
		
		push_eventp_llist(temp_event, &current_events);
		temp = temp->next;
	}

}

void data_init() {
	current_time = time(NULL);

	/* Initialize rainbow to display to today from (rainbow_h - 3)/2 days ago */
	int earlier_bound_relative_days = (rainbow_h - 3)/2;
	int later_bound_relative_days = 0;
	
	struct tm earlier_tm = *time_to_tm_local(current_time -
		earlier_bound_relative_days*86400);
	struct tm later_tm = *time_to_tm_local(current_time -
		later_bound_relative_days*86400);

	earlier_bound_day = start_of_day(&earlier_tm);
	later_bound_day = end_of_day(&later_tm);

	/* Load color globals */
	make_member_group_hex_dicts();
	make_group_color_dict();

	/* Load event globals */
	make_current_events(get_events_between(earlier_bound_day, later_bound_day));

}

