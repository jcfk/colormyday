#include "colormyday.h"

void end_begin_disp() {
	/* End old event & begin new event */
	char* name = get_command();

	if (strcmp(name, "") == 0) {
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
	display_tick();
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
			end_begin_disp();
			break;
		case KEY_DOWN:
			exit_colormyday();
		case KEY_RESIZE:
			resize_colormyday();
			break;
	}

	enum cursor_movement movement;
	switch((char)key) {
		case 'h':
			movement = C_LEFT;
			cursor_move(movement);
			break;
		case 'j':
			movement = C_DOWN;
			cursor_move(movement);
			break;
		case 'k':
			movement = C_UP;
			cursor_move(movement);
			break;
		case 'l':
			movement = C_RIGHT;
			cursor_move(movement);
			break;
		case 'G':
			movement = C_BOTTOM;
			cursor_move(movement);
			break;
		case '0':
			movement = C_ZERO;
			cursor_move(movement);
			break;
		case '$':
			movement = C_DOLLAR;
			cursor_move(movement);
			break;

		case 'e':
			edit_selection();
			break;

	}

	pthread_mutex_unlock(&display_access);
	pthread_mutex_unlock(&variable_access);
}

/*
 * SCRIPT MODE
 */

void end_begin_script(char* name) {
	if (current_event.event.name != NULL) {
		struct display_event last = end_current_event();
		begin_event(name);
		printf("Begun event: %s (ended %s)\n", name, last.event.name);

	} else {
		begin_event(name);
		printf("Begun event: %s\n", name);

	}

}

void end_current_event_script() {
	if (current_event.event.name != NULL) {
		struct display_event last = end_current_event();
		char* t = event_duration(last.event.start_time, last.event.end_time);
		printf("Ended event: %s (duration %s)\n", last.event.name, t);
		
	} else {
		printf("No current event to end.\n");

	}

}

void args_handle(int argc, char* argv[]) {
	char* arg;
	int i = 1;
	while(i < argc) {
		arg = argv[i];

		i += 1;
		if (strcmp(arg, "begin") == 0) {
			if (i == argc) {
				printf("ERR: Please enter the name of the event you'd like to begin. For example:\n\n\t$ colormyday begin Exercise\n\t$ colormyday begin \"Side Project\"\n\n");

			} else {
				end_begin_script(argv[i]);

			}

		} else if (strcmp(arg, "end") == 0) {
			end_current_event_script();

		}

	}
}

