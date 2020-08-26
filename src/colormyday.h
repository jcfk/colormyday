#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <ncurses.h>

#define CMD_DIRECTORY_NAME ".colormyday"
#define CMD_DB_NAME "data"

/* structs */
struct event {
	int start_time, end_time;
	char* name;
	char* note;
};

struct display_event {
	char* group;
	int color;
	struct event event;
};

struct charp_llist {
	char* content;
	struct charp_llist* next;
};

struct charpint_llist {
	char* c_content;
	int i_content;
	struct charpint_llist* next;
};

struct charpcharp_llist {
	char* content_1;
	char* content_2;
	struct charpcharp_llist* next;
};

struct eventp_llist {
	struct event event;
	struct eventp_llist* next;
};

struct display_eventp_llist {
	struct display_event display_event;
	struct display_eventp_llist* next;
};

enum cursor_movement {
	C_LEFT,
	C_DOWN,
	C_UP,
	C_RIGHT,
	C_TOP,
	C_BOTTOM,
	C_ZERO,
	C_DOLLAR,
	C_W,
	C_B
};

enum rainbow_scroll {
	R_UP,
	R_DOWN
};

/* globals */
extern pthread_mutex_t display_access, variable_access;
extern int current_time;
extern int earlier_bound_day, later_bound_day;
extern struct display_eventp_llist* current_events;
extern struct display_event current_event;
extern struct display_event cursor_event;
extern bool cursor_ticking;
extern bool thread_exit;

/* colormyday.c */
void exit_colormyday();
void resize_colormyday();

/* tick.c */
void* tick_init(void* arg);

/* input.c */
void input_handle(int key);
void args_handle_curses(char** args);
void args_handle_script(int argc, char* argv[]);

/* data.c */
void reload_current_events();
struct display_event time_to_event(int time);
void make_current_event(struct event event);
void data_init(int rainbow_h);
int make_color(char* code);
void make_group_color_dict();
void begin_event(char* name);
struct display_event end_current_event();
void scroll_current_events(enum rainbow_scroll direction);

/* display.c */
extern WINDOW* top_data;
extern WINDOW* rainbow;
extern WINDOW* bottom_data;
extern WINDOW* controls;

void display_events();
void cursor_move(enum cursor_movement movement);
void display_tick();
char* get_command();
void command();
void display_duration(struct display_event display_event);
void disp_event(struct display_event display_event);
void display_end_event(struct display_event display_event);
void cursor_init();
int windows_init();
void display_init();
void display_note(struct display_event display_event);


/* io.c */
char* cursor_event_path();
void make_member_group_hex_dicts(struct charpcharp_llist** member_group_dict, struct charpcharp_llist** group_hex_dict);
void make_dicts();
void io_init();
void event_to_file(struct event event);
struct event file_to_event(char* file);
struct charp_llist* get_events_between(int earlier_bound, int later_bound);
struct charint_llist* get_color_dict();

/* utils.c */
char** split_args(char* string);
int end_of_day(struct tm* tm);
int start_of_day(struct tm* tm);
char* event_duration(int start_time, int end_time);
struct tm* time_to_tm_local(int time);

void push_charp_llist(char* name, struct charp_llist** list);
void push_charpcharp_llist(char* content_1, char* content_2, struct charpcharp_llist** list);
void push_charpint_llist(char* c_content, int i_content, struct charpint_llist** list);
void push_eventp_llist(struct event event, struct eventp_llist** list);
void push_display_eventp_llist(struct display_event display_event, struct display_eventp_llist** list);

void free_charp_llist(struct charp_llist* list, bool malloced);
void free_display_eventp_llist(struct display_eventp_llist* list);

int charpint_dict(struct charpint_llist* list, char* c);
char* charpcharp_dict(struct charpcharp_llist* list, char* c);

void dump_charp_llist(struct charp_llist* list);
void dump_charpcharp_llist(struct charpcharp_llist* list);
void dump_charpint_llist(struct charpint_llist* list);
void dump_event(struct event event);
void dump_display_event(struct display_event display_event);
void dump_eventp_llist(struct eventp_llist* list);
void dump_cursor(int y, int x);
void print_cursor(int y, int x);

