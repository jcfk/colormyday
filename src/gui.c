#include "colormyday.h"
#include "gui.h"

short color10[3], color11[3];

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
static
void
free_curses(
	void
) {
	// anything?

}

void
end_curses(
	void
) {
	free_curses();
	if (has_colors()) {
		init_color(10, color10[0], color10[1], color10[2]);
		init_color(11, color11[0], color11[1], color11[2]);
	}
	endwin();
}

static
void
free_gui(
	void
) {
	if (current_events) {
		free_dsp_eventp_llist(current_events);
		current_events = NULL;
		current_event = NULL;
	}
	if (member_group_dict) {
		free_stringstring_llist(member_group_dict, true, true);
		member_group_dict = NULL;
	}
	if (group_hex_dict) {
		free_stringstring_llist(group_hex_dict, false, true);
		group_hex_dict = NULL;
	}
	if (group_color_dict) {
		free_stringint_llist(group_color_dict, false);
		group_color_dict = NULL;
	}
	free(cmddata_path);
	free(cmdconfig_path);
	free(cmdgroups_path);
}

void
exit_from_gui(
	int status
) {
	end_curses();
	free_gui();
	exit(status);
}

/**********************************************************************
* RESIZER *************************************************************
***********************************************************************/
void
resize(
	struct error** err
) {
	int rainbow_h = curses_windows_init(err);
	if ERRP return;
	gui_data_init(rainbow_h, err);
	if ERRP return;
	curses_start();
}

/*********************************************************************
 * INITIALIZER *******************************************************
 *********************************************************************/
static 
void 
save_default_colors(
	void
) {
	color_content(10, &color10[0], &color10[1], &color10[2]);
	color_content(11, &color11[0], &color11[1], &color11[2]);
}

static 
void 
curses_configure(
	struct error** err
) {
	initscr();
	cbreak();
	noecho();
	clear();
	curs_set(0);
	set_escdelay(10);

	if (has_colors() == FALSE) {
		init_error(err, NO_TERMINAL_COLORS, FATAL, 
			"Your terminal can't display colors.");
		return;
	}
	start_color();
	save_default_colors();
	use_default_colors();
	init_color(10, 0, 0, 0); // new black
	init_color(11, 1000, 1000, 1000); // new white
	init_color(1, 1000, 0, 1000); // weekend color
	init_pair(3, 1, -1); // weekend pair
	init_pair(4, COLOR_RED, -1); // error color
	init_pair(1, 10, 11);
	init_pair(2, 11, 10);
}

void
main_gui(
	struct error** err
) {
	/* Configure curses */
	curses_configure(err);
	if ERRP return;
	int rainbow_h = curses_windows_init(err);
	if ERRP return;

	/* Make globals */
	gui_data_init(rainbow_h, err);
	if ERRP return;

	curses_start();

	/* Begin tick thread */ 
	curses_tick();

	/* Wait for input */
	struct error* inner_err = NULL;
	int key;
	while (1) {
		key = wgetch(rainbow);
		input_handle(key, &inner_err);
		if (inner_err) {
			catch_err_gui(inner_err);
			inner_err = NULL;
		}
	}
}

