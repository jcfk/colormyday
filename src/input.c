#include "colormyday.h"

void key_up_handler() {
	/* End old event & begin new event */
	pthread_mutex_lock(&variable_access);
	pthread_mutex_lock(&display_access);

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
	display_end_event(current_event);

	if (cursor_ticking) {
		cursor_event = current_event;
		display_note(cursor_event);
		cursor_tick();
	}

	pthread_mutex_unlock(&display_access);
	pthread_mutex_unlock(&variable_access);
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
