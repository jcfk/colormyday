#include "colormyday.h"

short color10[3], color11[3];
pthread_mutex_t display_access, variable_access;
bool thread_exit;

/*
 * Function: save_default_colors
 * 
 * Saves terminal color values for colors 10 and 11.
 *
 */
static 
void 
save_default_colors(
	void
) {
	color_content(10, &color10[0], &color10[1], &color10[2]);
	color_content(11, &color11[0], &color11[1], &color11[2]);

}

/*
 * Function: curses_init
 *
 * Initializes curses and new colors and pairs.
 *
 */
static 
void 
curses_init(
	void
) {
	initscr();
	cbreak();
	noecho();
	clear();
	curs_set(0);

	if (has_colors() == FALSE) {
		endwin();
		exit(1);

	}

	start_color();
	save_default_colors();
	use_default_colors();

	/* new black */
	init_color(10, 0, 0, 0);
	/* new white */
	init_color(11, 1000, 1000, 1000);

	/* weekend color */
	init_color(1, 1000, 0, 1000);
	init_pair(3, 1, -1);

	/* error color */
	init_pair(4, COLOR_RED, -1);

	init_pair(1, 10, 11);
	init_pair(2, 11, 10);

	/* Eliminate delay after hitting esc */
	set_escdelay(10);

}

/*
 * Function: exit_colormyday
 *
 * Resets terminal colors and exits curses.
 *
 */
void
exit_colormyday(
	void
) {
	free_data();

	init_color(10, color10[0], color10[1], color10[2]);
	init_color(11, color11[0], color11[1], color11[2]);

	thread_exit = true;

	/* pthread_mutex_unlock(&display_access); */
	/* pthread_mutex_unlock(&variable_access); */

	/* reset_color_pairs(); */

	endwin();

	exit(0);

}

/*
 * Function: resize_colormyday
 *
 * Effectively restarts the app in response to a window resize.
 *
 */
void
resize_colormyday(
	void
) {
	int rainbow_h = windows_init();
	data_init(rainbow_h);
	display_init();

}

/*
 * Function: main
 *
 * In:
 *  argc
 *  argv
 *
 * If using curses, initializes screen and data and starts ticking, 
 * otherwise executes in scripting mode.
 *
 */
int
main(
	int argc, 
	char* argv[]
) {
	setlocale(LC_ALL, "");

	/* argument handling: turn this into its own function which 
	 * takes pointers to optargs */
	char* directory = NULL;
	int flag;
	while (true) {
		static struct option long_options[] = {
			{"directory", required_argument, 0, 1},
			{0, 0, 0, 0}

		};

		int option_index = 0;

		flag = getopt_long(argc, argv, "", long_options, &option_index);

		if (flag == -1) {
			break;

		}

		switch (flag) {
			case 1:
				directory = optarg;
				break;

		}
	}

	/* input/output */
	io_init(directory);

	/* configure window */
	curses_init();
	int rainbow_h = windows_init();

	/* data */
	data_init(rainbow_h);

	/* display */
	display_init();

	/* tick thread */ 
	pthread_mutex_init(&display_access, NULL);

	pthread_mutex_init(&variable_access, NULL);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_t tick_thread;
	pthread_create(&tick_thread, &attr, tick_init, NULL);

	/* keyboard input */
	int key;
	for(;;) {
		key = wgetch(rainbow);

		input_handle(key);

	}

	endwin();

	return(0);

}
