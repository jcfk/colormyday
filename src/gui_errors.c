#include "colormyday.h"
#include "gui.h"

static
void
print_err_curses(
	struct error* err
) {
	char* status_string = error_status_to_string(err->status);
	printf("Error %s: %s\n", status_string, err->message);
}

static
void
nonfatal_err(
	struct error* err
) {
	char* status_string = error_status_to_string(err->status);
	char* display_message = NULL;
	asprintf(&display_message, "Error %s: %s", status_string, err->message);
	display_error(display_message);
	free(display_message);
}

void
catch_err_gui(
	struct error* err
) {
	if (err) {
		if (err->fatal == FATAL) { // fix
			end_curses();
			print_err_curses(err);
			free_error(err);
			exit(EXIT_FAILURE);
		} else if (err->fatal == NONFATAL) { // fix
			pthread_mutex_lock(&globals_display);
			nonfatal_err(err);
			free_error(err);
			pthread_mutex_unlock(&globals_display);
		}
	}
}

