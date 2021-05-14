#include "colormyday.h"
#include "gui.h"

int               term_h;
int               term_w;
int               top_data_x, top_data_y, top_data_h, top_data_w;
int               rainbow_x, rainbow_y, rainbow_h, rainbow_w;
int               bottom_data_x, bottom_data_y, bottom_data_h, bottom_data_w;
int               controls_x, controls_y, controls_h, controls_w;
int               color_w, color_h;
double            slot_duration;
int               cursor[2];
int               time_cursor[2];
bool              cursor_ticking;
struct dsp_event* cursor_event;
WINDOW*           top_data;
WINDOW*           rainbow;
WINDOW*           bottom_data;
WINDOW*           controls;

/******************************************************************************************
* SCREEN POSITION CALCULATION *************************************************************
*******************************************************************************************/
static 
int 
start_y_bound(
	int time
) {
	struct tm temp_tm_earlier = earlier_bound_day;
	time_t temp_t = time;
	struct tm* temp_tm = localtime(&temp_t);
	temp_tm_earlier.tm_isdst = temp_tm->tm_isdst;
	return floor((time - mktime(&temp_tm_earlier) + slot_duration/2)/86400);
}

static 
int 
start_x_bound(
	int time
) {
	time_t temp = time;
	struct tm* temp_tm = localtime(&temp);
	int clock_sec = temp_tm->tm_sec + 60*temp_tm->tm_min + 3600*temp_tm->tm_hour;
	/* modulo is a temporary fix for overshooting the end-of-day */
	return ((int) floor(clock_sec/slot_duration + 0.5)) % color_w;
}

static 
int 
end_y_bound(
	int time
) {
	struct tm temp_tm_earlier = earlier_bound_day;
	time_t temp_t = time;
	struct tm* temp_tm = localtime(&temp_t);
	temp_tm_earlier.tm_isdst = temp_tm->tm_isdst;
	return floor((time - mktime(&temp_tm_earlier) - slot_duration/2)/86400);
}

static 
int 
end_x_bound(
	int time
) {
	time_t temp = time;
	struct tm* temp_tm = localtime(&temp);
	int clock_sec = temp_tm->tm_sec + 60*temp_tm->tm_min + 3600*temp_tm->tm_hour;
	return ((int) floor(clock_sec/slot_duration - 0.5) + color_w) % color_w;
}

static 
void 
set_bounds(
	int start_time, 
	int end_time, 
	int* start_x, 
	int* start_y, 
	int* end_x,
	int* end_y
) {
	if (start_time < mktime(&earlier_bound_day)) {
		*start_y = 0;
		*start_x = 0;
	} else {
		*start_y = start_y_bound(start_time);
		*start_x = start_x_bound(start_time);
	}
	if (end_time == -1) {
		if (time_cursor[0] > color_h - 1) {
			*end_y = color_h - 1;
			*end_x = color_w - 1;
		} else {
			*end_y = time_cursor[0];
			*end_x = time_cursor[1];
		}
	} else {
		*end_y = end_y_bound(end_time);
		*end_x = end_x_bound(end_time);
	}
}

/**********************************************************************
* DRAWERS *************************************************************
***********************************************************************/
void 
display_error(
	char* string
) {
	werase(controls);
	wattron(controls, COLOR_PAIR(4));
	wattron(controls, A_BOLD);
	mvwprintw(controls, 0, 0, string);
	wattroff(controls, A_BOLD);
	wattroff(controls, COLOR_PAIR(4));
	wrefresh(controls);
}

static 
void 
new_hour_marks(
	void
) {
	char* hour_list[24] = {
		"00","01","02","03","04","05",
		"06","07","08","09","10","11",
		"12","13","14","15","16","17",
		"18","19","20","21","22","23"
	};
	int width = rainbow_w - 7;
	int interval_w = width/24;
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
		mvwprintw(rainbow, 0, mark, hour_list[i]);
		i2 = 0;
		while (i2 < interval_w - 2) {
			mvwaddch(rainbow, 0, mark+2+i2, ' ');
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

static 
void 
new_date_marks(
	void
) {
	int later_time = mktime(&later_bound_day);
	color_h = 0;
	struct tm temp = earlier_bound_day;
	char date[7];
	int i = 0;
	int c;
	while (mktime(&temp) <= later_time) {
		if (temp.tm_mday == 1) {
			c = 3;
			strftime(date, 7, "%b %d", &temp);
		} else {
			c = 0;
			strftime(date, 7, "%a %d", &temp);
		}
		wattron(rainbow, COLOR_PAIR(c));
		mvwprintw(rainbow, i+2, 0, date);
		wattroff(rainbow, COLOR_PAIR(c));
		i = i + 1;
		temp = tm_add_interval(temp, 0, 0, 1);
		color_h = color_h + 1;
	}
	wrefresh(rainbow);
}

static 
void 
clear_rainbow(
	void
) {
	int start_y = 0;
	int start_x = 0;

	while (start_y*color_w + start_x <= color_w * color_h) {
		mvwaddch(rainbow, start_y+2, start_x+8, ' ');

		if (start_x > color_w) {
			start_y = start_y + 1;
			start_x = 0;

		} else {
			start_x = start_x + 1;

		}
	}

	wrefresh(rainbow);

}

void 
display_note(
	struct dsp_event* dsp_event
) {
	/* Default display values */
	char* title;
	char* note = "(null)";
	int color = 0; // or -1?

	if (dsp_event == NULL) {
		asprintf(&title, "C: (null)");
	} else {
		/* Get event data */
		int start_time = dsp_event->event->start_time;
		int end_time = dsp_event->event->end_time;
		char* group = dsp_event->group;
		char* name = dsp_event->event->name;
		note = dsp_event->event->note;
		color = dsp_event->color;

		/* struct tm* temp_tm = malloc(sizeof(struct tm)); */
		/* char start_time_english[50]; */
		/* time_t temp_t = start_time; */
		/* localtime_r(&temp_t, temp_tm); */
		/* strftime(start_time_english, sizeof(start_time_english), "%a, %H:%M:%S", temp_tm); */
		/* Generate note title */
		char* start_time_english = time_to_dsp_string(start_time);
		if (end_time == TILL_NOW) {
			asprintf(&title, "C: %s (group: %s) | %s -> NOW", name, group, start_time_english);
		} else {
			/* char end_time_english[50]; */
			/* temp_t = end_time; */
			/* localtime_r(&temp_t, temp_tm); */
			/* strftime(end_time_english, sizeof(end_time_english), "%a, %H:%M:%S", temp_tm); */
			char* end_time_english = time_to_dsp_string(end_time);
			char* duration = event_duration(start_time, end_time);
			asprintf(
				&title, 
				"C: %s (group: %s) | %s -> %s | Duration: %s", 
				name, 
				group, 
				start_time_english, 
				end_time_english, 
				duration
			);
			free(duration);
			free(end_time_english);
		}
		free(start_time_english);
		/* free(temp_tm); */
	}

	/* Draw note */
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

static
void 
display_duration(
	struct dsp_event* dsp_event
) {
	char* temp;
	int color = 0;

	/* Create title */
	if (dsp_event == NULL) {
		asprintf(&temp, "Current: (no event)");
	} else {
		char* name = dsp_event->event->name;
		int diff = current_time - dsp_event->event->start_time;
		float hours = ((float) diff)/3600;
		int minutes = 60*(hours - floor(hours));
		asprintf(&temp, "Current: %s - %dh %dm", name, (int) floor(hours), minutes);
		color = dsp_event->color;
	}
	
	/* Draw title */
	werase(top_data);
	box(top_data, 0, 0);
	mvwprintw(top_data, 1, 1, CMD_VERSION);
	wattron(top_data, COLOR_PAIR(color));
	wattron(top_data, A_BOLD);
	mvwprintw(top_data, 1, (top_data_w - (int) strlen(temp))/2, temp);
	wattroff(top_data, COLOR_PAIR(color));
	wattroff(top_data, A_BOLD);
	wrefresh(top_data);
	free(temp);
}

static
void 
disp_event(
	struct dsp_event* dsp_event, 
	bool clear
) {
	int start_time = dsp_event->event->start_time;
	int end_time = dsp_event->event->end_time;
	if (mktime(&later_bound_day) < end_time) {
		end_time = mktime(&later_bound_day);
	}

	int start_x, start_y, end_x, end_y;
	set_bounds(start_time, end_time, &start_x, &start_y, &end_x, &end_y);
	if (clear) {
		while (start_x + color_w*start_y <= end_x + color_w*end_y) {
			if (start_y == cursor[0] && start_x == cursor[1]) {
				mvwprintw(rainbow, start_y+2, start_x+8, "C");
			} else {
				mvwprintw(rainbow, start_y+2, start_x+8, " ");
			}
			start_y = start_y + ((start_x + 1) / color_w);
			start_x = (start_x + 1) % color_w;	
		}
	} else {
		char* name = dsp_event->event->name;
		char* note = dsp_event->event->note;
		int color = dsp_event->color;
		int len_name = strlen(name);
		int len_title;
		if STRNEQ(note, "-1") {
			asprintf(&name, "%s-%s", name, note);
			len_title = strlen(name);
		} else {
			asprintf(&name, "%s ", name);
			len_title = len_name + 1;
		}
		char* namep = name;
		wattron(rainbow, COLOR_PAIR(color));
		wattron(rainbow, A_BOLD);
		int i = 0;
		bool printing_name = true;
		while (start_x + color_w*start_y <= end_x + color_w*end_y) {
			if (start_y == cursor[0] && start_x == cursor[1]) {
				wattron(rainbow, A_BOLD);
				mvwprintw(rainbow, start_y+2, start_x+8, "C");
				if (i >= len_name) {
					wattroff(rainbow, A_BOLD);
				}
				cursor_event = dsp_event;
			} else if (start_y == end_y && start_x == end_x) {
				mvwprintw(rainbow, start_y+2, start_x+8, "\u2591");
				break;
			} else if (i < len_name) {
				mvwaddch(rainbow, start_y+2, start_x+8, *name);
			} else if (i < len_title) {
				if (printing_name) {
					wattroff(rainbow, A_BOLD);
					printing_name = false;
				}
				mvwaddch(rainbow, start_y+2, start_x+8, *name);
			} else {
				mvwprintw(rainbow, start_y+2, start_x+8, "\u223c");
			}
			++name;
			i = i + 1;
			start_y = start_y + ((start_x + 1) / color_w);
			start_x = (start_x + 1) % color_w;	
		}
		wattroff(rainbow, A_BOLD);
		wattroff(rainbow, COLOR_PAIR(color));
		free(namep);
	}
	wrefresh(rainbow);
}

void 
display_events(
	void
) {
	struct dsp_eventp_llist* temp = current_events;
	while (temp) {
		disp_event(temp->dsp_event, false);
		temp = temp->next;
	}
}

/**************************************************************************
* SCREEN DATA *************************************************************
***************************************************************************/
static 
struct dsp_event*
first_visible_event(
	void
) {
	if (current_events) {
		struct dsp_eventp_llist* temp = current_events;
		while (temp->next) {
			temp = temp->next;

		}

		return temp->dsp_event;

	} else {
		return NULL;

	}
}

static 
struct dsp_event*
last_visible_event(
	void
) {
	if (current_events) {
		return current_events->dsp_event; // this means that current_events is stored with recents at the top
	} else {
		return NULL;
	}
}

static 
struct dsp_event* 
forward_event(
	void
) {
	/* int start_x, start_y; */
	struct dsp_event* ret = NULL;
	struct dsp_eventp_llist* temp = current_events;
	while (temp) {
		int start_x = start_x_bound(temp->dsp_event->event->start_time);
		int start_y = start_y_bound(temp->dsp_event->event->start_time);
		if (start_x + start_y*color_w > cursor[1] + cursor[0]*color_w) {
			ret = temp->dsp_event;
			temp = temp->next;
		} else {
			return ret;
		}
	}
	return ret;
}

static 
struct dsp_event* 
backward_event(
	void
) {
	int start_x, start_y;
	struct dsp_eventp_llist* temp = current_events;
	while (temp) {
		start_x = start_x_bound(temp->dsp_event->event->start_time);
		start_y = start_y_bound(temp->dsp_event->event->start_time);
		if (start_x + start_y*color_w >= cursor[1] + cursor[0]*color_w) {
			temp = temp->next;
		} else {
			return temp->dsp_event;
		}
	}
	return NULL;
}

static 
struct dsp_event*
cursor_to_event(
	int cursor_y, 
	int cursor_x
) {
	struct dsp_eventp_llist* current_events_iter = current_events;
	int cursor_dist = cursor_y*color_w + cursor_x;
	struct event* temp;
	int start_x, start_y, end_x, end_y;
	int start;
	int end;
	while (current_events_iter) {
		temp = current_events_iter->dsp_event->event;
		set_bounds(temp->start_time, temp->end_time, &start_x, &start_y, &end_x, &end_y);
		start = start_y*color_w + start_x;
		end = end_y*color_w + end_x;
		if (start <= cursor_dist && cursor_dist <= end) {
			return current_events_iter->dsp_event;
		}
		current_events_iter = current_events_iter->next;
	}
	return NULL;
}

/*********************************************************************
 * INPUT *************************************************************
 *********************************************************************/
static 
void 
scrolling(
	enum rainbow_scroll direction,
	struct error** err
) {
	if (direction == R_UP) {
		earlier_bound_day = tm_add_interval(earlier_bound_day, 0, 0, -1);
		later_bound_day = tm_add_interval(later_bound_day, 0, 0, -1);
		time_cursor[0] = time_cursor[0] + 1;
	} else {
		earlier_bound_day = tm_add_interval(earlier_bound_day, 0, 0, 1);
		later_bound_day = tm_add_interval(later_bound_day, 0, 0, 1);
		time_cursor[0] = time_cursor[0] - 1;
	}
	reload_current_events(err);
	if ERRP return;

	clear_rainbow();
	box(rainbow, 0, 0);
	new_hour_marks();
	new_date_marks();
	display_events();
}

char* 
get_command(
	void
) {
	/* Draw command input */
	char* disp = ":\u2588";
	werase(controls);
	mvwprintw(controls, 0, 0, disp);
	wrefresh(controls);

	/* Record command */
	char* command = malloc(2);
	command[0] = '\0';
	int key, len;
	bool reading = true;
	while(reading) {
		key = wgetch(controls);
		switch(key) {
			case 27: // ESC
				free(command);
				command = "";
				reading = false;
				break;
			case 10: // ENTER
				reading = false;
				break;
			case KEY_BACKSPACE: // BACKSPACE
				/* Trim command */
				len = strlen(command);
				if (len > 0) {
					command[len-1] = '\0';
				}

				/* Display change */
				asprintf(&disp, ":%s\u2588", command);
				werase(controls);
				mvwprintw(controls, 0, 0, disp);
				wrefresh(controls);
				free(disp);
				break;
			default: ;
				/* Extend command */
				len = strlen(command);
				command = realloc(command, len+2);
				command[len] = (char)key;
				command[len+1] = '\0';
				len = len + 1;
				
				/* Display change */
				asprintf(&disp, ":%s\u2588", command);
				werase(controls);
				mvwprintw(controls, 0, 0, disp);
				wrefresh(controls);
				free(disp);
				break;
		}
	}
	werase(controls);
	wrefresh(controls);
	return command;
}

void 
cursor_move(
	enum cursor_movement movement,
	struct error** err
) {
	/* Calculate new cursor position */
	int new_y = cursor[0];
	int new_x = cursor[1];
	struct dsp_event* temp;
	int start_time, end_time, start_time_temp;
	bool will_scroll = false;
	enum rainbow_scroll direction;
	switch (movement) {
		case C_NOTHIN:
			return;
		case C_UP:
			if (new_y-1 >= 0) {
				new_y = new_y - 1;
			} else {
				will_scroll = true;
				direction = R_UP;
				/* scrolling(direction, err); */
				/* if ERRP return; */
			}
			break;
		case C_DOWN:
			if (new_y+1 < color_h) {
				new_y = new_y + 1;
			} else {
				will_scroll = true;
				direction = R_DOWN;
				/* scrolling(direction, err); */
				/* if ERRP return; */
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
				if (new_y+1 < color_h) {
					new_y = new_y + 1;
					new_x = 0;
				}
			}
			break;
		case C_TOP: ;
			start_time = -1; // why -1?
			struct dsp_event* first_event = first_visible_event();
			if (first_event) {
				start_time = first_event->event->start_time;
				if (start_time >= mktime(&earlier_bound_day)) {
					new_y = start_y_bound(start_time);
					new_x = start_x_bound(start_time);
				} else {
					new_y = 0;
					new_x = 0;
				}
			} else {
				new_y = 0;
				new_x = 0;
			}
			break;
		case C_BOTTOM: ;
			struct dsp_event* last_event = last_visible_event();
			if (last_event) {
				end_time = last_event->event->end_time;
				if (end_time > mktime(&later_bound_day)) {
					new_y = color_h-1;
					new_x = color_w-1;
				} else if (end_time == TILL_NOW) {
					new_y = time_cursor[0];
					new_x = time_cursor[1];
				} else {
					new_y = end_y_bound(end_time); // account for TILL_NOW
					new_x = end_x_bound(end_time);
					if (new_x == -1) { // -1???
						new_y = new_y - 1;
						new_x = color_w - 1;
					}
				}
			} else {
				if (current_time > mktime(&later_bound_day)) {
					new_y = color_h - 1;
					new_x = color_w - 1;
				} else {
					new_y = time_cursor[0];
					new_x = time_cursor[1];
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
			if (temp) { 
				start_time_temp = temp->event->start_time;
				new_y = start_y_bound(start_time_temp);
				new_x = start_x_bound(start_time_temp);
			}
			break;
		case C_B: ;
			temp = backward_event();
			if (temp) {
				start_time_temp = temp->event->start_time;
				if (start_time_temp >= mktime(&earlier_bound_day)) { 
					new_y = end_y_bound(start_time_temp);
					new_x = start_x_bound(start_time_temp);
				} else {
					new_y = 0;
					new_x = 0;
				}
			}
			break;
	}

	/* Cursor ticks if it's at the end */
	if ((new_y == time_cursor[0]) && (new_x == time_cursor[1])) {	
		cursor_ticking = true;
	} else {
		cursor_ticking = false;
	}

	/* Remove old cursor */
	if (will_scroll) {
		cursor_event = NULL; // wow, this is hackish (fix later)
		scrolling(direction, err);
		if ERRP return;
	} else {
		mvwprintw(rainbow, 1, cursor[1]+8, " ");
		mvwprintw(rainbow, cursor[0]+2, 7, " ");
		mvwprintw(rainbow, 1, new_x+8, "\u25bc");
		mvwprintw(rainbow, new_y+2, 7, "\u25b6");
		if (cursor_event == NULL) {
			mvwaddch(rainbow, cursor[0]+2 , cursor[1]+8, ' ');
			cursor[0] = new_y;
			cursor[1] = new_x;
		} else {
			cursor[0] = new_y;
			cursor[1] = new_x;
			disp_event(cursor_event, false); // fix
		}
		cursor_event = cursor_to_event(cursor[0], cursor[1]);
	}

	/* Draw new cursor */
	if (cursor_event == NULL) {
		mvwaddch(rainbow, cursor[0]+2, cursor[1] + 8, 'C');
	} else {
		disp_event(cursor_event, false);
	}
	display_note(cursor_event);
}

/*******************************************************************
* TICK *************************************************************
********************************************************************/
void 
display_tick(
	struct error** err
) {
	/* Calculate new cursor position */
	time_cursor[0] = end_y_bound(current_time);
	time_cursor[1] = end_x_bound(current_time);

	if (cursor_ticking) {
		/* Scroll down if cursor ticks offscreen */
		if (time_cursor[0] > color_h-1) {
			enum rainbow_scroll direction = R_DOWN;
			scrolling(direction, err);
			if ERRP return;
		}

		/* Update cursor position */
		cursor[0] = time_cursor[0];
		cursor[1] = time_cursor[1];

		/* Draw new cursor */
		mvwprintw(rainbow, 1, cursor[1]+8, " ");
		mvwprintw(rainbow, cursor[0]+2, 7, " ");
		mvwprintw(rainbow, 1, cursor[1]+8, "\u25bc");
		mvwprintw(rainbow, cursor[0]+2, 7, "\u25b6");
		wrefresh(rainbow);

		cursor_event = cursor_to_event(cursor[0], cursor[1]);
		/* disp_event(cursor_event, false); */
		display_note(current_event);
	}

	display_duration(current_event);
	display_events();
}

/***************************************************************************
* INITIALIZERS *************************************************************
****************************************************************************/
static
void 
cursor_init(
	void
) {
	cursor_ticking = true;

	/* Calculate cursor position */
	time_cursor[0] = end_y_bound(current_time);
	time_cursor[1] = end_x_bound(current_time);
	if (time_cursor[1] == -1) {
		time_cursor[0] = time_cursor[0] - 1;
		time_cursor[1] = color_w - 1;
	}

	/* Draw cursor position */
	mvwprintw(rainbow, 1, cursor[1]+8, " ");
	mvwprintw(rainbow, cursor[0]+2, 7, " ");
	cursor[0] = time_cursor[0];
	cursor[1] = time_cursor[1];
	mvwaddch(rainbow, cursor[0]+2, cursor[1]+8, 'C');
	mvwprintw(rainbow, 1, cursor[1]+8, "\u25bc");
	mvwprintw(rainbow, cursor[0]+2, 7, "\u25b6");
	wrefresh(rainbow);
}

void 
curses_start(
	void
) {
	/* Draw basics */
	new_hour_marks();
	new_date_marks();
	cursor_init();

	/* Draw events */
	display_events();
	display_duration(current_event);
	display_note(cursor_event); // who initialized cursor_event?
	wmove(rainbow, 0, 0);
	wrefresh(rainbow);
}

int 
curses_windows_init(
	struct error** err
) {
	term_h = LINES;
	term_w = COLS;

	if ((term_h < MIN_TERMINAL_H) || (term_w < MIN_TERMINAL_W)) {
		init_error(err, TOO_SMALL_TERMINAL, FATAL,
			"Minimum dimensions are %dx%d", MIN_TERMINAL_H, MIN_TERMINAL_W);
		return 0;
	}

	top_data_x = 0;
	top_data_y = 0;
	top_data_h = TOP_DATA_H;
	top_data_w = term_w;

	rainbow_x = 0;
	rainbow_y = top_data_h;
	rainbow_h = term_h - TOP_DATA_H - BOTTOM_DATA_H - CONTROLS_H;
	rainbow_w = term_w;

	bottom_data_x = 0;
	bottom_data_y = top_data_h + rainbow_h;
	bottom_data_h = BOTTOM_DATA_H;
	bottom_data_w = term_w;
	
	controls_x = 0;
	controls_y = top_data_h + rainbow_h + bottom_data_h;
	controls_h = CONTROLS_H;
	controls_w = term_w;

	top_data = newwin(top_data_h, top_data_w, top_data_y, top_data_x);
	box(top_data, 0, 0);
	rainbow = newwin(rainbow_h, rainbow_w, rainbow_y, rainbow_x);
	box(rainbow, 0, 0);
	bottom_data = newwin(bottom_data_h, bottom_data_w, bottom_data_y, bottom_data_x);
	box(bottom_data, 0, 0);
	controls = newwin(controls_h, controls_w, controls_y, controls_x);

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


