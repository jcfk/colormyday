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

	if (cursor_ticking) {
		cursor_event = current_event;
		display_note(cursor_event);
		cursor_tick();
	}
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
		case 'e':
			edit_selection();
			break;
	}

	pthread_mutex_unlock(&display_access);
	pthread_mutex_unlock(&variable_access);
}

