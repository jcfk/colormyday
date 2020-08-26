#include "colormyday.h"

/* terminal size */
int term_h;
int term_w;

/* window sizes */
int top_data_x, top_data_y, top_data_h, top_data_w;
int rainbow_x, rainbow_y, rainbow_h, rainbow_w;
int bottom_data_x, bottom_data_y, bottom_data_h, bottom_data_w;
int controls_x, controls_y, controls_h, controls_w;

/* actual size of color bar */
int color_w, color_h;

/* time represented by one character */
double slot_duration;

/* cursor */
int cursor[2];
int time_cursor[2];
bool cursor_ticking;
struct display_event cursor_event;

WINDOW* top_data;
WINDOW* rainbow;
WINDOW* bottom_data;
WINDOW* controls;

/*
 * SCAFFOLDING DISPLAYER
 */
void new_hour_marks() {
	char* hour_list[24] = {"00","01","02","03","04","05",
		"06","07","08","09","10","11",
		"12","13","14","15","16","17",
		"18","19","20","21","22","23"};
	int width = rainbow_w - 7;
	int interval_w = width / 24;
	int shift = 8;
	color_w = interval_w * 24;
	slot_duration = 86400/(double) color_w;

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

	free(a);
	free(b);
	free(c);

	wrefresh(rainbow);
}

void new_date_marks() {
	int earlier_time = earlier_bound_day;
	int later_time = later_bound_day;

	color_h = 0;

	char date[7];
	char day[4];
	time_t temp;
	int i = 0;
	int c;
	while (earlier_time <= later_time) {
		temp = earlier_time;
		strftime(day, 4, "%a", localtime(&temp));
		strftime(date, 7, "%a %d", localtime(&temp));

		if ((strcmp(day, "Sat") == 0) || (strcmp(day, "Sun") == 0)) {
			c = 3;

		} else {
			c = 0;

		}

		wattron(rainbow, COLOR_PAIR(c));
		mvwprintw(rainbow, 3+i, 0, date);
		wattroff(rainbow, COLOR_PAIR(c));
		i = i + 1;
		earlier_time = earlier_time + 86400;

		color_h = color_h + 1;
	}

	wrefresh(rainbow);

}

void clear_rainbow() {
	int start_y = 0;
	int start_x = 0;

	while (start_y*color_w + start_x <= color_w * color_h) {
		mvwaddch(rainbow, start_y+3, start_x+8, ' ');

		if (start_x > color_w) {
			start_y = start_y + 1;
			start_x = 0;

		} else {
			start_x = start_x + 1;

		}
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

	char* title = NULL;

	if (name == NULL) {
		asprintf(&title, "C: (null)");
		note = "(null)";
		color = 0;

	} else {
		char start_time_english[50];
		struct tm teeem_s = *time_to_tm_local(start_time);
		strftime(start_time_english, sizeof(start_time_english), "%a, %H:%M:%S", &teeem_s);

		if (end_time == -1) {
			asprintf(&title, "C: %s (group: %s) | %s -> NOW", name, group, start_time_english);

		} else {
			struct tm teeem_e = *time_to_tm_local(end_time);
			char end_time_english[50];
			strftime(end_time_english, sizeof(end_time_english), "%a, %H:%M:%S", &teeem_e);

			char* t = event_duration(start_time, end_time);

			asprintf(&title, "C: %s (group: %s) | %s -> %s | Duration: %s", name, group, start_time_english, end_time_english, t);

			free(t);

		}
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

	free(title);

}

/*
 * EVENT DISPLAYER
 */
int left_x_bound(int time) {
	float a, b, ret;
	a = (time - earlier_bound_day) % 86400;
	b = slot_duration;
	ret = a / b;

	if (round(ret) <= ret) {
		ret = floor(ret);

	} else {
		ret = floor(ret) + 1;

	}

	return ret;

}

int right_x_bound(int time) {
	float a, b, ret;
	a = (time - earlier_bound_day) % 86400;
	b = slot_duration;
	ret = a / b;

	if (round(ret) <= ret) {
		ret = floor(ret) - 1;

	} else {
		ret = floor(ret);

	}

	return ret;

}

int y_bound(int time) {
	return floor((time - earlier_bound_day)/86400);

}

int over_x(int diff) {
	float a, b, ret;
	a = diff % 86400;
	b = slot_duration;
	ret = a/b;

	return floor(ret);
}

void display_duration(struct display_event display_event) {
	char* temp;
	char* name = display_event.event.name;
	int c;

	if (name == NULL) {
		asprintf(&temp, "Current: (no event)");
		c = -1;

	} else {
		int diff = current_time - display_event.event.start_time;
		float hours = ((float) diff)/3600;

		asprintf(&temp, "Current: %s - %.2f hrs", name, hours);

		c = display_event.color;
	}
	
	werase(top_data);
	box(top_data, 0, 0);
	mvwprintw(top_data, 1, 1, "COLORMYDAY");
	wattron(top_data, COLOR_PAIR(c));
	wattron(top_data, A_BOLD);
	mvwprintw(top_data, 1, (top_data_w - (int) strlen(temp))/2, temp);
	wattroff(top_data, COLOR_PAIR(c));
	wattroff(top_data, A_BOLD);
	wrefresh(top_data);

	free(temp);

}

void disp_event(struct display_event display_event) {
	/* Get display position */
	int start_x, start_y, end_x, end_y;
	
	int start_time = display_event.event.start_time;
	int end_time = display_event.event.end_time;
	char* name = display_event.event.name;

	if (name == NULL) {
		return;

	}

	if (start_time < earlier_bound_day) {
		start_y = 0;
		start_x = 0;

	} else {
		start_y = y_bound(start_time);
		start_x = left_x_bound(start_time);

	}
	
	if (later_bound_day < end_time) {
		end_time = later_bound_day;

	}

	if (end_time == -1) {
		if (time_cursor[0] > color_h - 1) {
			end_y = color_h - 1;
			end_x = color_w - 1;

		} else {
			end_y = time_cursor[0];
			end_x = time_cursor[1];

		}

	} else {
		end_y = y_bound(end_time);
		end_x =right_x_bound(end_time);

		if (end_x == -1) {
			end_y = end_y - 1;
			end_x = color_w - 1;

		}
	}

	/* Actually display event */
	int len = strlen(name);

	char* note = display_event.event.note;
	int c = display_event.color;

	asprintf(&name, "%s-%s", name, note);
	int len2 = strlen(name);
	char* namep = name;

	wattron(rainbow, COLOR_PAIR(c));
	wattron(rainbow, A_BOLD);
	int i = 0;
	bool printing_name = true;
	while (start_x + color_w*start_y <= end_x + color_w*end_y) {
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
			mvwprintw(rainbow, start_y+3, start_x+8, "\u223c");

		}

		++name;
		i = i + 1;
		
		start_y = start_y + ((start_x + 1) / color_w);
		start_x = (start_x + 1) % color_w;	

	}

	wattroff(rainbow, A_BOLD);
	wattroff(rainbow, COLOR_PAIR(c));

	wrefresh(rainbow);

	free(namep);

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
 * SCREEN DATA
 */
struct display_event first_visible_event() {
	struct display_eventp_llist* temp = current_events;
	while (temp->next) {
		temp = temp->next;

	}

	return temp->display_event;

}

struct display_event last_visible_event() {
	return current_events->display_event;

}

/* gets event after cursor event */
struct display_event* forward_event() {
	struct display_eventp_llist* temp = current_events;
	struct display_event* ret = NULL;
	while (temp) {
		if (temp->display_event.event.start_time == cursor_event.event.start_time) {
			return ret;

		} else {
			ret = &(temp->display_event);
			temp = temp->next;

		}
	}

	return NULL;
}

/* gets event prior to cursor event */
struct display_event* backward_event() {
	struct display_eventp_llist* temp = current_events;
	while (temp) {
		if (temp->display_event.event.start_time == cursor_event.event.start_time) {
			break;

		} else {
			temp = temp->next;

		}
	}

	if (temp == NULL) {
		return NULL;

	}

	if (temp->next != NULL) {
		return &(temp->next->display_event);

	} else {
		return NULL;

	}
}

struct display_event cursor_to_event(int cursor_y, int cursor_x) {
	int new_time = earlier_bound_day + cursor_y * 86400 + cursor_x * slot_duration + (slot_duration / 2);

	return time_to_event(new_time);

}

/*
 * INPUT
 */
void scrolling(enum rainbow_scroll direction) {
	if (direction == R_UP) {
		earlier_bound_day = earlier_bound_day - 86400;
		later_bound_day = later_bound_day - 86400;

		time_cursor[0] = time_cursor[0] + 1;

	} else {
		earlier_bound_day = earlier_bound_day + 86400;
		later_bound_day = later_bound_day + 86400;

		time_cursor[0] = time_cursor[0] - 1;

	}

	scroll_current_events(direction);

	new_date_marks();

	clear_rainbow();

	display_events();

}

char* get_command() {
	char* disp = ":\u2588";

	mvwprintw(controls, 1, 1, disp);
	wrefresh(controls);

	char* ret = malloc(2);
	ret[0] = 0;
	int key, len;
	bool reading = true;
	while(reading) {
		key = wgetch(controls);

		if (ret == NULL) {
			len = 0;

		} else {
			len = strlen(ret);

		}

		switch(key) {
			/* ESC */
			case 27:
				if (ret != NULL) {
					free(ret);
					ret = NULL;
				}

				reading = false;
				break;

			/* ENTER */
			case 10:
				reading = false;
				break;

			/* BACKSPACE */
			case KEY_BACKSPACE:
				if (len == 1) { 
					free(ret);
					ret = NULL;

				} else if (len > 1) {
					ret[len - 1] = 0;

				}

				if (ret == NULL) {
					asprintf(&disp, ":\u2588");

				} else {
					asprintf(&disp, ":%s\u2588", ret);

				}

				werase(controls);
				box(controls, 0, 0);
				mvwprintw(controls, 1, 1, disp);
				wrefresh(controls);

				free(disp);
				break;
			
			default: ;
				if (ret == NULL) {
					ret = malloc(2);
					ret[0] = 0;

				}

				ret = realloc(ret, len + 2);
				ret[len] = (char)key;
				ret[len + 1] = 0;

				asprintf(&disp, ":%s\u2588", ret);
				mvwprintw(controls, 1, 1, disp);
				wrefresh(controls);

				free(disp);

		}
	}

	werase(controls);
	box(controls, 0, 0);
	wrefresh(controls);

	return ret;

}

void command() {
	char* request = get_command();

	if (request == NULL) {
		mvwprintw(top_data, 1, 20, "null");
		wrefresh(top_data);
		return;

	}

	if (*request == 'q') {
		exit_colormyday();

	}

	char** args = split_args(request);

	/* 
	int i = 0;
	while (args[i + 1]) {
		i = i + 1;

	}

	char* n;
	asprintf(&n, "%d", i);
	mvwprintw(top_data, 1, 20, n);
	wrefresh(top_data);
	free(n);
	*/

	args_handle_curses(args);

	free(args);
	free(request);

}

void cursor_move(enum cursor_movement movement) {
	int new_y = cursor[0];
	int new_x = cursor[1];

	int start_time, end_time;
	struct display_event* temp;
	switch (movement) {
		case C_UP:
			if (new_y-1 >= 0) {
				new_y = new_y - 1;

			} else {
				enum rainbow_scroll direction = R_UP;
				scrolling(direction);

			}

			break;

		case C_DOWN:
			if (new_y+1 < color_h) {
				new_y = new_y + 1;

			} else {
				enum rainbow_scroll direction = R_DOWN;
				scrolling(direction);

			}

			break;

		case C_LEFT:
			if (new_x-1 >= 0) {
				new_x = new_x - 1;

			} else {
				if (new_y-1 >= 0) {
					new_y = new_y - 1;
					new_x = color_w - 1;

				}
			}

			break;

		case C_RIGHT:
			if (new_x+1 < color_w) {
				new_x = new_x + 1;

			} else {
				new_y = new_y + 1;
				new_x = 0;

			}

			break;

		case C_TOP: ;
			start_time = first_visible_event().event.start_time;

			if (start_time >= earlier_bound_day) { 
				new_y = y_bound(start_time);
				new_x = left_x_bound(start_time);

			} else {
				new_y = 0;
				new_x = 0;

			}

			break;

		case C_BOTTOM: ;
			end_time = last_visible_event().event.end_time;

			if (end_time == -1) {
				new_y = time_cursor[0];
				new_x = time_cursor[1];

			} else if (end_time > later_bound_day) {
				new_y = color_h-1;
				new_x = color_w-1;

			} else {
				new_y = y_bound(end_time);
				new_x = right_x_bound(end_time);

				if (new_x == -1) {
					new_y = new_y - 1;
					new_x = color_w - 1;

				}
			}

			break;

		case C_ZERO:
			new_x = 0; 

			break;

		case C_DOLLAR:
			new_x = color_w - 1;

			break;

		case C_W: ;
			temp = forward_event();		

			if (temp != NULL) { 
				int start_time = temp->event.start_time;
				new_y = y_bound(start_time);
				new_x = left_x_bound(start_time);

			}

			break;

		case C_B: ;
			temp = backward_event();

			if (temp != NULL) {
				int start_time = temp->event.start_time;
				if (start_time >= earlier_bound_day) { 
					new_y = y_bound(start_time);
					new_x = left_x_bound(start_time);

				} else {
					new_y = 0;
					new_x = 0;

				}
			}
			
			break;

	}

	if ((new_y == time_cursor[0]) && (new_x == time_cursor[1])) {	
		cursor_ticking = true;

	} else {
		cursor_ticking = false;

	}

	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");
	mvwprintw(rainbow, 2, new_x + 8, "\u25bc");
	mvwprintw(rainbow, new_y + 3, 7, "\u25b6");

	if (cursor_event.event.name == NULL) {
		mvwaddch(rainbow, cursor[0] + 3 , cursor[1] + 8, ' ');

	} else {
		cursor[0] = new_y;
		cursor[1] = new_x;

		disp_event(cursor_event);

	}

	cursor[0] = new_y;
	cursor[1] = new_x;
	cursor_event = cursor_to_event(cursor[0], cursor[1]);

	if (cursor_event.event.name == NULL) {
		mvwaddch(rainbow, cursor[0] + 3, cursor[1] + 8, 'C');

	} else {
		disp_event(cursor_event);


	}

	display_note(cursor_event);

}

/*
 * TICK
 */
void cursor_tick() {
	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");

	cursor[0] = time_cursor[0];
	cursor[1] = time_cursor[1];

	mvwprintw(rainbow, 2, cursor[1] + 8, "\u25bc");
	mvwprintw(rainbow, cursor[0] + 3, 7, "\u25b6");

	wrefresh(rainbow);

	disp_event(cursor_event);

	cursor_event = cursor_to_event(cursor[0], cursor[1]);

	display_note(current_event);

}

void display_tick() {
	time_cursor[0] = y_bound(current_time);
	time_cursor[1] = right_x_bound(current_time);

	if (time_cursor[1] == -1) {
		time_cursor[0] = time_cursor[0] - 1;
		time_cursor[1] = color_w - 1;

	}

	if (cursor_ticking && (time_cursor[0] > color_h - 1)) {
		enum rainbow_scroll direction = R_DOWN;
		scrolling(direction);

	}

	if (cursor_ticking) {
		cursor_tick();

	}

	mvwprintw(top_data, 1, 1, "COLORMYDAY");
	display_duration(current_event);
	disp_event(current_event);

}

/*
 * INITIALIZERS
 */
void top_data_init() {
	mvwprintw(top_data, 1, 1, "COLORMYDAY");
	wrefresh(top_data);

}

void scaffolding_init() {
	new_hour_marks();
	new_date_marks();

}

void cursor_init() {
	cursor_ticking = true;

	time_cursor[0] = y_bound(current_time);
	time_cursor[1] = right_x_bound(current_time);

	if (time_cursor[1] == -1) {
		time_cursor[0] = time_cursor[0] - 1;
		time_cursor[1] = color_w - 1;

	}

	mvwprintw(rainbow, 2, cursor[1] + 8, " ");
	mvwprintw(rainbow, cursor[0] + 3, 7, " ");

	cursor[0] = time_cursor[0];
	cursor[1] = time_cursor[1];

	mvwaddch(rainbow, cursor[0] + 3, cursor[1] + 8, 'C');

	mvwprintw(rainbow, 2, cursor[1] + 8, "\u25bc");
	mvwprintw(rainbow, cursor[0] + 3, 7, "\u25b6");

	wrefresh(rainbow);

}

void display_init() {
	top_data_init();

	scaffolding_init();	

	cursor_init();

	display_events();

	display_duration(current_event);
	
	display_note(cursor_event);

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
