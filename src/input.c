#include "colormyday.h"

/*
 * CURSES MODE
 */

/* 
 * Function: begin_handle
 *
 * In:
 *  name: string containing name of new event
 *  late_time: string containing timestamp of when the new event began, or null
 *
 * This function is for use in curses mode. It sets the end of the 
 * current event (if there is one) to time T, and creates and sets 
 * the beginning of a new event to time T. Time T is the late time 
 * if one is given, or else it is the current time.
 *
 */
char*
begin_handle(
	char* name, 
	char* late_time
) {
	if (strcmp(name, "") == 0) {
		return("no event name given");

	}

	int late_time_seconds = 0;
	if (late_time) {
		late_time_seconds = string_to_time(late_time);

		if (late_time_seconds == -1) {
			return("invalid --late time");

		}

	}

	/* If current event exists end current_event */
	if (current_event.event.name != NULL) {
		disp_event(&current_event, true);
		struct display_event* temp = end_current_event(late_time_seconds);
		/* disp_event(temp, false); */

	}

	char* temp_name;
	asprintf(&temp_name, "%s", name);
	struct display_event* temp = begin_event(temp_name, late_time_seconds);
	/* disp_event(temp, false); */

	display_tick();

	return(NULL);

}

/*
 * Function: end_event
 *
 * This function ends the current event.
 *
 */
void
end_event(
) {
	end_current_event(0);

	reload_current_events();
	display_events();

	display_tick();

}

/*
 * Function: edit_selection
 *
 * This function executes a shell command which launches an editor 
 * on the path of the cursor event. Upon exit, all visible events
 * are reloaded and rerendered.
 *
 */
void 
edit_selection(
) {
	endwin();

	char* command = NULL;
	asprintf(&command, "vi %s", cursor_event_path());
	int err = system(command);

	if (err == -1) {
		error("%s", "error opening file");

	}

	reload_current_events();
	display_events();
	display_note(cursor_event);

	free(command);

}

/*
 * Function: args_begin
 *
 * In:
 *  args: a split-on-" " list of command arguments
 *
 * This function parses arguments to a :begin command.
 *
 */
char*
args_begin(
	char** args
) {
	char* name;
	char* late_time = NULL;

	int i = 1;
	char* token;
	bool late = false;
	while (args[i]) {
		token = args[i];

		if (late) {
			late = false;
			late_time = token;

		} else {
			if (strcmp(token, "--late") == 0) {
				late = true;

			} else {
				name = token;

			}
		}

		i = i + 1;

	}

	char* err = begin_handle(name, late_time);

	if (err) {
		return(err);

	}

	return(NULL);

}

/*
 * Function: route_args
 *
 * In:
 *  args: a split-on-" " list of command arguments
 *
 * This function sends the given args to the corresponding command's
 * argument parser, or calls a function directly if the command is
 * simple enough.
 *
 */
void 
route_args(
	char** args
) {
	char* cmd;
	cmd = args[0];

	for(int i = 0; cmd[i]; i++) {
		cmd[i] = tolower(cmd[i]);

	}

	char* err = NULL;
	if (strcmp(cmd, "begin") == 0) {
		err = args_begin(args);

	} else if (strcmp(cmd, "end") == 0) {
		/* err = */ 
		end_event();

	} else if (strcmp(cmd, "edit") == 0) {
		/* err = */ 
		edit_selection();

	} else {
		error("%s", "unknown command");

	}

	if (err) {
		error("%s", err);

	}
}

/*
 * Function: input_handle
 *
 * In:
 *  key: an integer corresponding to the key pressed
 *
 * This function calls the function corresponding to the keypress.
 * It reserves thread access to global data during execution.
 *
 */
void 
input_handle(
	int key
) {
	pthread_mutex_lock(&variable_access);
	pthread_mutex_lock(&display_access);
		
	switch(key) {
		case KEY_RESIZE:
			resize_colormyday();
			break;
	}

	enum cursor_movement movement;
	switch((char)key) {
		/* vim keys */
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
		case 'g':
			movement = C_TOP;
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
		case 'w':
			movement = C_W;
			cursor_move(movement);
			break;
		case 'b':
			movement = C_B;
			cursor_move(movement);
			break;

		/* special keys */
		case ':':
			command();
			break;

	}

	pthread_mutex_unlock(&display_access);
	pthread_mutex_unlock(&variable_access);

}


