#include "colormyday.h"

short color0[3];
short color1[3];
short color2[3];
short color3[3];
short color10[3];
short color11[3];

void save_default_colors() {
	color_content(0, &color0[0], &color0[1], &color0[2]);
	color_content(1, &color1[0], &color1[1], &color1[2]);
	color_content(2, &color2[0], &color2[1], &color2[2]);
	color_content(3, &color3[0], &color3[1], &color3[2]);
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
	save_default_colors();
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
	init_color(0, color0[0], color0[1], color0[2]);
	init_color(1, color1[0], color1[1], color1[2]);
	init_color(2, color2[0], color2[1], color2[2]);
	init_color(3, color3[0], color3[1], color3[2]);
	init_color(10, color10[0], color10[1], color10[2]);
	init_color(11, color11[0], color11[1], color11[2]);
	endwin();
	exit(0);
}

void resize_colormyday() {
	windows_init();
	data_init();
	display_init();
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	/* input/output */
	io_init();

	/* configure window */
	curses_init();
	windows_init();

	/* data */
	data_init();

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
	pthread_mutex_lock(&display_access);
	wmove(rainbow, 0, 0);
	wrefresh(rainbow);
	pthread_mutex_unlock(&display_access);
	
	int key;
	for(;;) {
		key = wgetch(rainbow);

		switch(key) {
			case KEY_UP:
				key_up();
				break;
			case KEY_DOWN:
				exit_colormyday();
			case KEY_RESIZE:
				resize_colormyday();
				break;
		}

		/* switch((char)key) {
			case 'h':
				move_right();
				break;
			case 'j':
				move_down();
				break;
			case 'k':
				move_up();
				break;
			case 'l':
				move_left();
				break;
		} */

		mvwaddch(bottom_data, 5, 5, (char)key);
		wrefresh(bottom_data);
	}

	endwin();

	return(0);
}
