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
int time_cursor[2];
bool cursor_ticking;
struct display_event cursor_event;

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

void cursor_move(enum cursor_movement movement) {
	int new_y = cursor[0];
	int new_x = cursor[1];
	
	switch (movement) {
		case UP:
			new_y = new_y - 1;
			break;
		case DOWN:
			new_y = new_y + 1;
			break;
		case LEFT:
			new_x = new_x - 1;
			break;
		case RIGHT:
			new_x = new_x + 1;
			break;
		case BOTTOM:
			new_y = time_cursor[0];
			new_x = time_cursor[1];
			break;

	}
	
	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");

	cursor[0] = new_y;
	cursor[1] = new_x;

	mvwprintw(rainbow, 2, cursor[1] + 8, "\u25bc");
	mvwprintw(rainbow, cursor[0] + 3, 7, "\u25b6");
	
	if (cursor[0] == time_cursor[0] && cursor[1] == time_cursor[1]) {	
		cursor_ticking = true;
	} else {
		cursor_ticking = false;
	}

	disp_event(cursor_event);

	int new_time = earlier_bound_day + new_y * 86400 + new_x * slot_duration + (slot_duration / 2);
	cursor_event = time_to_event(new_time);	

	disp_event(cursor_event);

	display_note(cursor_event);
	
}

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
 * NOTE DISPLAYER
 */
void display_note(struct display_event display_event) {
	int start_time = display_event.event.start_time;
	int end_time = display_event.event.end_time;
	char* name = display_event.event.name;
	char* note = display_event.event.note;
	char* group = display_event.group;
	int color = display_event.color;
	
	struct tm teeem_s = *time_to_tm_local(start_time);
	char start_time_english[50];
	strftime(start_time_english, sizeof(start_time_english), "%a, %H:%M:%S", &teeem_s);

	char* title = NULL;
	if (end_time == -1) {
		asprintf(&title, "C: %s (group: %s) | %s -> NOW", name, group, start_time_english);

	} else {
		struct tm teeem_e = *time_to_tm_local(end_time);
		char end_time_english[50];
		strftime(end_time_english, sizeof(end_time_english), "%a, %H:%M:%S", &teeem_e);
		
		asprintf(&title, "C: %s (group: %s) | %s -> %s", name, group, start_time_english, end_time_english);

	}

	werase(bottom_data);
	box(bottom_data, 0, 0);

	wattron(bottom_data, A_BOLD);
	wattron(bottom_data, COLOR_PAIR(color));
	mvwprintw(bottom_data, 1, 1, title);
	wattroff(bottom_data, A_BOLD);
	wattroff(bottom_data, COLOR_PAIR(color));
	mvwprintw(bottom_data, 2, 1, "--- NOTES ---");
	mvwprintw(bottom_data, 3, 1, note);
	wrefresh(bottom_data);
}

/*
 * EVENT DISPLAYERS
 */
int left_x_bound(int diff) {
	float a, b, ret;
	a = diff % 86400;
	b = slot_duration;
	ret = a / b;

	if (round(ret) < ret) {
		return floor(ret);
	} else {
		return floor(ret) + 1;
	}
}

int right_x_bound(int diff) {
	float a, b, ret;
	a = diff % 86400;
	b = slot_duration;
	ret = a / b;

	if (round(ret) < ret) {
		return floor(ret) - 1;
	} else {
		return floor(ret);
	}
}

void display_duration(struct display_event display_event) {
	char* temp;
	char* name = display_event.event.name;

	int diff = current_time - display_event.event.start_time;
	float hours = ((float) diff)/3600;

	asprintf(&temp, "Current: %s - %.2f hrs", name, hours);

	int c = display_event.color;

	wattron(top_data, COLOR_PAIR(c));
	wattron(top_data, A_BOLD);
	mvwprintw(top_data, 1, (top_data_w - (int) strlen(temp))/2, temp);
	wattroff(top_data, COLOR_PAIR(c));
	wattroff(top_data, A_BOLD);
	wrefresh(top_data);

}

void disp_event(struct display_event display_event) {
	/* Get display position */
	char* name;
	int start_time, end_time;
	int start_x, start_y, end_x, end_y;
	int diff;
	
	start_time = display_event.event.start_time;
	end_time = display_event.event.end_time;
	name = display_event.event.name;

	if (end_time == -1) {
		end_time = current_time;
	}

	start_y = 0;
	start_x = 0;

	if (earlier_bound_day < start_time) {
		diff = start_time - earlier_bound_day;
		start_y = floor(diff/86400);
		start_x = left_x_bound(diff);
	
	}

	end_y = 0;
	end_x = 0;

	diff = end_time - earlier_bound_day;
	end_y = end_y + floor(diff/86400);
	end_x = right_x_bound(diff);

	/* Actually display event */
	int len = strlen(name);

	char* group = display_event.group;
	int c = display_event.color;

	asprintf(&name, "%s-%s", name, group);
	int len2 = strlen(name);

	wattron(rainbow, COLOR_PAIR(c));
	wattron(rainbow, A_BOLD);
	int i = 0;
	bool printing_name = true;
	while (start_x + color_width*start_y <= end_x + color_width*end_y) {
		if (start_y == cursor[0] && start_x == cursor[1]) {
			wattron(rainbow, A_BOLD);
			mvwprintw(rainbow, start_y+3, start_x+8, "C");

			if (i > len) {
				wattroff(rainbow, A_BOLD);
			}

			cursor_event = display_event;

		} else if (start_y == end_y && start_x == end_x) {
			mvwprintw(rainbow, start_y+3, start_x+8, "\u2591");
			break;

		} else if (i < len) {
			mvwaddch(rainbow, start_y+3, start_x+8, *name);
	
		} else if (i < len2) {
			if (printing_name) {
				wattroff(rainbow, A_BOLD);
				printing_name = false;
			}

			mvwaddch(rainbow, start_y+3, start_x+8, *name);

		} else {
			mvwaddch(rainbow, start_y+3, start_x+8, '=');

		}

		++name;
		i = i + 1;
		
		start_y = start_y + ((start_x + 1) / color_width);
		start_x = (start_x + 1) % color_width;	

	}
	wattroff(rainbow, COLOR_PAIR(c));

	wrefresh(rainbow);
}

void display_events() {
	struct display_eventp_llist* temp = current_events;

	struct display_event temp_display_event;
	while (temp) {
		temp_display_event = temp->display_event;
		disp_event(temp_display_event);

		temp = temp->next;
	}
}

/*
 * TICK
 */
void display_tick() {
	int diff = current_time - earlier_bound_day;

	if (current_event.event.name != NULL) {
		display_duration(current_event);
		disp_event(current_event);
	}

	time_cursor[0] = floor(diff/86400);
	time_cursor[1] = right_x_bound(diff);

	if (cursor_ticking) {
		cursor_tick();
	} 

}

void cursor_tick() {
	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");

	cursor[0] = time_cursor[0];
	cursor[1] = time_cursor[1];

	mvwprintw(rainbow, 2, cursor[1] + 8, "\u25bc");
	mvwprintw(rainbow, cursor[0] + 3, 7, "\u25b6");

	wrefresh(rainbow);

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

	display_tick();

}

void display_init() {
	top_data_init();

	rainbow_init();	

	cursor_init();

	display_events();

	if (current_event.event.name != NULL) {
		display_duration(current_event);
	}
	
	if (cursor_event.event.name != NULL) {
		display_note(cursor_event);
	}

	wmove(rainbow, 0, 0);
	wrefresh(rainbow);
}

int windows_init() {
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

	return rainbow_h;
}
