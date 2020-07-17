#include "colormyday.h"

int term_h;
int term_w;
int top_data_x, top_data_y, top_data_h, top_data_w;
int rainbow_x, rainbow_y, rainbow_h, rainbow_w;
int bottom_data_x, bottom_data_y, bottom_data_h, bottom_data_w;
int controls_x, controls_y, controls_h, controls_w;
int color_width;
double slot_duration;
int cursor[2];

WINDOW* top_data;
WINDOW* rainbow;
WINDOW* bottom_data;
WINDOW* controls;

char* get_command() {
	mvwprintw(controls, 1, 1, "(name):");
	wrefresh(controls);

	echo();

	char* ret = malloc(50);
	mvwgetnstr(controls, 1, 10, ret, 50);

	noecho();

	werase(controls);
	box(controls, 0, 0);
	wrefresh(controls);

	return ret;
}

/* void cursor_move(enum cursor_movement movement) {
	int new_y, new_x;

	switch(movement) {
		case LEFT:
			new_y = cursor[0];
			if (cursor[1] - 1 < 1) {
				new_y = new_y - 1;
				new_x = color_width - 1;
			} else {
				new_x = cursor[1] - 1;
			}

		case DOWN:
			new_x = cursor[1];
			if (cursor[0] + 1 ) {
				
				
			}

	}

	if (cursor_ticking) {
		cursor_ticking = false;
	}

	selected_event = time_to_event(

} */

/*
 * SCAFFOLDING DISPLAYERS
 */
void new_hour_marks() {
	char* hour_list[24] = {"00","01","02","03","04","05",
		"06","07","08","09","10","11",
		"12","13","14","15","16","17",
		"18","19","20","21","22","23"};
	int width = rainbow_w - 7;
	int interval_w = width / 24;
	int shift = 8;
	color_width = interval_w * 24;
	slot_duration = 86400/(double) color_width;

	int i = 0;
	int i2;
	int mark;
	int color;
	wattron(rainbow, A_BOLD);
	while (i < 24) {
		if (i % 2 == 0) {
			color = 2;
		} else {
			color = 1;
		}

		mark = shift+interval_w*i;
		wattron(rainbow, COLOR_PAIR(color));
		mvwprintw(rainbow, 1, mark, hour_list[i]);
		
		i2 = 0;
		while (i2 < interval_w - 2) {
			mvwaddch(rainbow, 1, mark+2+i2, ' ');
			i2 = i2 + 1;
		}

		wattroff(rainbow, COLOR_PAIR(color));
		i = i + 1;
		
	}
	wattroff(rainbow, A_BOLD);
	
	char *a, *b, *c;
	asprintf(&a, "%d", width);
	asprintf(&b, "%d", interval_w);
	asprintf(&c, "%d", term_w);

	wrefresh(rainbow);
}

void new_date_marks() {
	int earlier_time = earlier_bound_day;
	int later_time = later_bound_day;
	
	char date[7];
	char day[4];
	time_t temp;
	int i = 0;
	while (earlier_time <= later_time) {
		temp = earlier_time;
		strftime(day, 4, "%a", localtime(&temp));
		strftime(date, 7, "%a %d", localtime(&temp));

		int c = 0;
		if ((strcmp(day, "Sat") == 0) || (strcmp(day, "Sun") == 0)) {
			c = 3;
		}

		wattron(rainbow, COLOR_PAIR(c));
		mvwprintw(rainbow, 3+i, 0, date);
		wattroff(rainbow, COLOR_PAIR(c));
		i = i + 1;
		earlier_time = earlier_time + 86400;
	}

	wrefresh(rainbow);
}

/*
 * EVENT DISPLAYERS
 */
void display_duration(struct event event) {
	char* temp;
	char* name = event.name;

	int diff = current_time - event.start_time;
	float hours = ((float) diff)/3600;

	asprintf(&temp, "Current: %s - %.2f hrs", name, hours);

	int c = 1;
	char* group = NULL;
	group = charpcharp_dict(member_group_dict, name);
	if (group) {
		c = charpint_dict(group_color_dict, group);
	}

	wattron(top_data, COLOR_PAIR(c));
	wattron(top_data, A_BOLD);
	mvwprintw(top_data, 1, (top_data_w - (int) strlen(temp))/2, temp);
	wattroff(top_data, COLOR_PAIR(c));
	wattroff(top_data, A_BOLD);
	wrefresh(top_data);

}

void display_event(struct event event) {
	/* Get display position */
	char* name;
	int start_time, end_time;
	int start_x, start_y, end_x, end_y;
	int diff;
	int temp_x;
	
	start_time = event.start_time;
	end_time = event.end_time;
	name = event.name;

	start_y = 0;
	start_x = 0;

	if (earlier_bound_day < start_time) {
		diff = start_time - earlier_bound_day;
		start_y = start_y + floor(diff/86400);
	
		temp_x = (diff % 86400)/slot_duration;
		if (round(temp_x) < temp_x) {
			start_x = start_x + floor(temp_x) + 1;
		} else {
			start_x = start_x + floor(temp_x);
		}
	}

	end_y = 0;
	end_x = 0;

	diff = end_time - earlier_bound_day;
	end_y = end_y + floor(diff/86400);
	temp_x = (diff % 86400)/slot_duration;

	if (round(temp_x) <= temp_x) {
		end_x = end_x + floor(temp_x) - 1;
	} else {
		end_x = end_x + floor(temp_x);
	}

	/* Actually display event */
	int len = strlen(name);

	char* group = NULL;
	group = charpcharp_dict(member_group_dict, name);

	int c = 1;
	if (group) {
		c = charpint_dict(group_color_dict, group);
	}

	asprintf(&name, "%s-%s", name, group);
	int len2 = strlen(name);

	wattron(rainbow, COLOR_PAIR(c));
	wattron(rainbow, A_BOLD);
	int i = 0;
	bool printing_name = true;
	while (start_x + color_width*start_y <= end_x + color_width*end_y) {
		if (start_y == end_y && start_x == end_x) {
			mvwprintw(rainbow, start_y+3, start_x+8, "\u2591");
			break;

		} else if (i < len) {
			mvwaddch(rainbow, start_y+3, start_x+8, *name);
			++name;

			i = i + 1;
			
		} else if (i < len2) {
			if (printing_name) {
				wattroff(rainbow, A_BOLD);
				printing_name = false;
			}

			mvwaddch(rainbow, start_y+3, start_x+8, *name);
			++name;
			
			i = i + 1;

		} else {
			mvwaddch(rainbow, start_y+3, start_x+8, '=');

		}
		
		start_y = start_y + ((start_x + 1) / color_width);
		start_x = (start_x + 1) % color_width;	

	}
	wattroff(rainbow, COLOR_PAIR(c));

	wrefresh(rainbow);
}

void display_end_event(struct event event) {	
	struct event temp = event;
	temp.end_time = current_time + slot_duration;
	
	display_event(temp);
	
}

void display_events() {
	struct eventp_llist* temp = current_events;

	struct event temp_event;
	while (temp) {
		temp_event = temp->event;

		if (temp_event.end_time == -1) {
			display_end_event(temp_event);
		} else {
			display_event(temp_event);
		}

		temp = temp->next;
	}
}

/*
 * TICK
 */
void cursor_tick() {

	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");
	
	if (current_event.name == NULL) {
		mvwprintw(rainbow, cursor[0] + 3, cursor[1] + 8, " ");
	}

	wrefresh(rainbow);

	cursor_init();
}

/*
 * INITIALIZERS
 */
void top_data_init() {
	mvwprintw(top_data, 1, 1, "COLORMYDAY");
	wrefresh(top_data);

}

void rainbow_init() {
	new_hour_marks();
	new_date_marks();
}

void cursor_init() {
	cursor_ticking = true;

	int y;
	int x;
	int diff = current_time - earlier_bound_day;
	
	y = floor(diff/86400);
	x = floor((diff % 86400)/slot_duration);
	cursor[0] = y;
	cursor[1] = x;

	mvwprintw(rainbow, 2, x + 8, "\u25bc");
	mvwprintw(rainbow, y + 3, 7, "\u25b6");

	int c = 1;
	char* group = NULL;
	if (current_event.name) {
		group = charpcharp_dict(member_group_dict, current_event.name);
		if (group) {
			c = charpint_dict(group_color_dict, group);
		}
	}

	wattron(rainbow, COLOR_PAIR(c));
	wattron(rainbow, A_BOLD);
	mvwprintw(rainbow, y + 3, x + 8, "C");
	wattroff(rainbow, COLOR_PAIR(c));
	wattroff(rainbow, A_BOLD);
	wrefresh(rainbow);
	
}

void display_init() {
	top_data_init();

	rainbow_init();	

	display_events();
	
	cursor_init();
	
	if (current_event.name != NULL) {
		display_duration(current_event);
	}
}

void windows_init() {
	term_h = (LINES - 1);
	term_w = COLS;

	top_data_x = 0;
	top_data_y = 0;
	top_data_h = 3;
	top_data_w = term_w;

	rainbow_x = 0;
	rainbow_y = top_data_h;
	rainbow_h = term_h - 14;
	rainbow_w = term_w;

	bottom_data_x = 0;
	bottom_data_y = top_data_h + rainbow_h;
	bottom_data_h = 9;
	bottom_data_w = term_w;
	
	controls_x = 0;
	controls_y = top_data_h + rainbow_h + bottom_data_h;
	controls_h = 3;
	controls_w = term_w;

	top_data = newwin(top_data_h, top_data_w, top_data_y, top_data_x);
	box(top_data, 0, 0);
	rainbow = newwin(rainbow_h, rainbow_w, rainbow_y, rainbow_x);
	box(rainbow, 0, 0);
	bottom_data = newwin(bottom_data_h, bottom_data_w, bottom_data_y, bottom_data_x);
	box(bottom_data, 0, 0);
	controls = newwin(controls_h, controls_w, controls_y, controls_x);
	box(controls, 0, 0);

	wrefresh(top_data);
	wrefresh(rainbow);
	wrefresh(bottom_data);
	wrefresh(controls);

	keypad(top_data, TRUE);
	keypad(rainbow, TRUE);
	keypad(bottom_data, TRUE);
	keypad(controls, TRUE);

}
