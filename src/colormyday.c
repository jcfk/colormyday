#include "colormyday.h"

short color10[3], color11[3];

void save_default_colors() {
	color_content(10, &color10[0], &color10[1], &color10[2]);
	color_content(11, &color11[0], &color11[1], &color11[2]);
}

void curses_init() {
	initscr();
	cbreak();
	noecho();
	/* nonl(); */
	clear();
	curs_set(0);

	if (has_colors() == FALSE) {
		endwin();
		exit(1);
	}
	start_color();
	save_default_colors(color10, color11);
	use_default_colors();

	/* new black */
	init_color(10, 0, 0, 0);
	/* new white */
	init_color(11, 1000, 1000, 1000);

	/* weekend color */
	init_color(1, 482, 0, 1000);
	init_pair(3, 1, -1);

	init_pair(1, 10, 11);
	init_pair(2, 11, 10);
}

void exit_colormyday() {
	init_color(10, color10[0], color10[1], color10[2]);
	init_color(11, color11[0], color11[1], color11[2]);
	endwin();
	exit(0);
}

void resize_colormyday() {
	int rainbow_h = windows_init();
	data_init(rainbow_h);
	display_init();
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");

	/* input/output */
	io_init();

	/* configure window */
	curses_init(color10, color11);
	int rainbow_h = windows_init();

	/* data */
	data_init(rainbow_h);

	/* display */	
	display_init();

	/* tick thread */ 
	pthread_mutex_t display_access;
	pthread_mutex_init(&display_access, NULL);

	pthread_mutex_t variable_access;
	pthread_mutex_init(&variable_access, NULL);

	pthread_t tick_thread;
	pthread_create(&tick_thread, NULL, tick_init, NULL);

	/* keyboard input */
	int key;
	for(;;) {
		key = wgetch(rainbow);

		input_handle(key);
	}

	endwin();

	return(0);
}
