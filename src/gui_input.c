#include "colormyday.h"
#include "gui.h"

/**********************************************************************
* ACTIONS *************************************************************
***********************************************************************/
static 
void
main_begin(
	char* name, 
	char* late_time,
	struct error** err
) {
	/* Parse late time */
	int late_time_seconds = current_time;
	if (late_time) {
		late_time_seconds = string_to_time(late_time, err);
		if ERRP return;
		validate_late_time(late_time_seconds, current_time, err);
		if ERRP return;
	}

	/* End current event */
	if (current_event) {
		end_current_event_gui(late_time_seconds, err);
		if ERRP return;
	}

	/* Begin new event */
	char* temp_name;
	asprintf(&temp_name, "%s", name); // why asprintf?
	begin_event_gui(temp_name, late_time_seconds, err);
	if ERRP return;
	display_tick(err);
	if ERRP return;
}

static
void
main_end(
	char* late_time,
	struct error** err
) {
	/* Parse late time */
	int late_time_seconds = current_time;
	if (late_time) {
		late_time_seconds = string_to_time(late_time, err);
		if ERRP return;
		validate_late_time(late_time_seconds, current_time, err);
		if ERRP return;
	}

	/* End current event */
	if (current_event) {
		end_current_event_gui(late_time_seconds, err);
		if ERRP return;
	} else {
		init_error(err, ALREADY_ENDED_EVENT, NONFATAL, NULL);
		return;
	}
}

static 
void 
main_edit(
	struct error** err
) {
	/* Open editor */
	endwin();
	char* event_path = cursor_event_path(err);
	if ERRP return;
	char* command = NULL;
	asprintf(&command, "%s %s", TEXT_EDITOR, event_path);
	int system_response = system(command);
	free(command);
	if (system_response == -1) {
		init_error(err, CANNOT_OPEN_FILE, NONFATAL, "path: %s", event_path);
		return;
	}
	
	/* Redraw gui */
	/* how does this recover from endwin? */
	reload_current_events(err);
	display_events();
	display_note(cursor_event);
}

/***************************************************************************
* COMMAND MODE *************************************************************
****************************************************************************/
static
void
exec_command(
	struct gui_options* opts,
	struct error** err
) {
	switch (opts->action) {
		case G_BEGIN:
			main_begin(opts->begin_name, opts->late_time, err);
			break;
		case G_END:
			main_end(opts->late_time, err);
			break;
		case G_EDIT:
			main_edit(err);
			break;
		case G_QUIT:
			exit_from_gui(EXIT_SUCCESS);
			break;
		default:
			break;
	}
	free_gui_options(opts);
	if ERRP return;
}

static
void 
main_command(
	struct error** err
) {
	/* Get argv and argc */
	char* request = get_command();
	if STREQ(request, "") return;
	char** argv = NULL;
	int argc = fill_argv(&argv, request);
	free(request);

	/* Parse options */
	struct gui_options* opts = malloc_gui_options();
	fill_gui_options(opts, argc, argv, err);
	free_argv(argc, argv);
	if ERRP return;

	exec_command(opts, err);
	if ERRP return;
}

/*******************************************************************************
* PRIMARY FUNCTION *************************************************************
********************************************************************************/
void 
input_handle(
	int key,
	struct error** err
) {
	pthread_mutex_lock(&globals_display);
		
	switch(key) {
		case KEY_RESIZE:
			resize(err);
			break;
	}

	enum cursor_movement movement = C_NOTHIN;
	switch((char)key) {
		case 'h':
			movement = C_LEFT;
			break;
		case 'j':
			movement = C_DOWN;
			break;
		case 'k':
			movement = C_UP;
			break;
		case 'l':
			movement = C_RIGHT;
			break;
		case 'g':
			movement = C_TOP;
			break;
		case 'G':
			movement = C_BOTTOM;
			break;
		case '0':
			movement = C_ZERO;
			break;
		case '$':
			movement = C_DOLLAR;
			break;
		case 'w':
			movement = C_W;
			break;
		case 'b':
			movement = C_B;
			break;
		case ':':
			main_command(err);
			break;
	}

	cursor_move(movement, err);
	pthread_mutex_unlock(&globals_display);
	if ERRP return; // should this go before or after unlocking?
}


