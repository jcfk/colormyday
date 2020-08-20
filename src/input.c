#include "colormyday.h"

void end_begin() {
	/* End old event & begin new event */
	char* name = get_command();

	if (strcmp(name, "") == 0) {
		pthread_mutex_unlock(&display_access);
		pthread_mutex_unlock(&variable_access);
		return;
	}

	if (current_event.event.name != NULL) {
		disp_event(end_current_event());
	}

	char* temp_name;
	asprintf(&temp_name, "%s", name);
	begin_event(temp_name);
	free(name);

	werase(top_data);
	box(top_data, 0, 0);
	mvwprintw(top_data, 1, 1, "COLORMYDAY");
	display_duration(current_event);
	disp_event(current_event);

	display_tick();
}

void move_right() {
	enum cursor_movement movement = RIGHT;
	cursor_move(movement);
}

void move_down() {
	enum cursor_movement movement = DOWN;
	cursor_move(movement);
}

void move_up() {
	enum cursor_movement movement = UP;
	cursor_move(movement);
}

void move_left() {
	enum cursor_movement movement = LEFT;
	cursor_move(movement);
}

void move_bottom() {
	enum cursor_movement movement = BOTTOM;
	cursor_move(movement);

}

void move_0() {
	enum cursor_movement movement = ZERO;
	cursor_move(movement);
}

void move_dollar() {
	enum cursor_movement movement = DOLLAR;
	cursor_move(movement);
}

void edit_selection() {
	endwin();

	char* command = NULL;
	asprintf(&command, "nano %s", cursor_event_path());
	system(command);

	reload_current_events();
	display_events();
	display_note(cursor_event);
}

void input_handle(int key) {
	pthread_mutex_lock(&variable_access);
	pthread_mutex_lock(&display_access);
		
	switch(key) {
		case KEY_UP:
			end_begin();
			break;
		case KEY_DOWN:
			exit_colormyday();
		case KEY_RESIZE:
			resize_colormyday();
			break;
	}

	switch((char)key) {
		case 'h':
			move_left();
			break;
		case 'j':
			move_down();
			break;
		case 'k':
			move_up();
			break;
		case 'l':
			move_right();
			break;
		case 'G':
			move_bottom();
			break;
		case '0':
			move_0();
			break;
		case '$':
			move_dollar();
			break;

		case 'e':
			edit_selection();
			break;

	}

	pthread_mutex_unlock(&display_access);
	pthread_mutex_unlock(&variable_access);
}

void args_handle(int argc, char* argv[]) {
	char* arg;
	int i = 1;
	while(i < argc) {
		arg = argv[i];

		if (strcmp(arg, "begin") == 0) {
			i += 1;
			if (i == argc) {
				printf("ERR: Please enter the name of the event you'd like to begin. For example:\n\n\t$ colormyday begin Exercise\n\t$ colormyday begin \"Side Project\"\n\n");

			} else {
				struct display_event last = end_current_event();
				begin_event(argv[i]);
				printf("Begun event: %s (ended %s)\n", argv[i], last.event.name);

			}

		} else if (strcmp(arg, "end") == 0) {
			struct display_event last = end_current_event();
			
			if (last.event.name == NULL) {
				printf("No current event to end.\n");

			} else {
				char* t = event_duration(last.event.start_time, last.event.end_time);

				printf("Ended event: %s (duration %s)\n", last.event.name, t);

			}

			i += 1;
		}
	}
}

