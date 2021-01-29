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
#include <stdarg.h>
#include <ctype.h>
#include <regex.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <ncurses.h>

#define CMD_DIRECTORY_NAME ".colormyday"
#define CMD_DB_NAME "data"

#define CMD_VERSION "COLORMYDAY v" VERSION

/* structs */
struct event {
	int start_time, end_time;
	char* name;
	char* note;
};

struct display_event {
	char* group;
	int color;
	int start[2];
	int end[2];
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
extern struct tm earlier_bound_day;
extern struct tm later_bound_day;
extern struct display_eventp_llist* current_events;
extern struct display_event current_event;
extern struct display_event cursor_event;
extern bool cursor_ticking;
extern bool thread_exit;

/* colormyday.c */
void exit_colormyday(void);
void resize_colormyday(void);

/* tick.c */
void* tick_init(void* arg);

/* input.c */
void input_handle(int key);
void route_args(char** args);

/* script.c */
void args_handle_script(int argc, char* argv[]);

/* data.c */
void reload_current_events(void);
struct display_event time_to_event(int time);
void make_current_event(struct event event);
void data_init(int rainbow_h);
int make_color(char* code);
void make_group_color_dict(struct charpint_llist** list);
struct display_event* begin_event(char* name, int late_time);
struct display_event* end_current_event(int late_time);
void scroll_current_events(enum rainbow_scroll direction);
void free_data(void);

/* display.c */
extern WINDOW* top_data;
extern WINDOW* rainbow;
extern WINDOW* bottom_data;
extern WINDOW* controls;

void debug(char* string, ...);
void error(char* string, ...);

void display_events(void);
void cursor_move(enum cursor_movement movement);
void display_tick(void);
char* get_command(void);
void command(void);
void display_duration(struct display_event display_event);
void disp_event(struct display_event* display_event, bool clear);
void display_end_event(struct display_event display_event);
void cursor_init(void);
int windows_init(void);
void display_init(void);
void display_note(struct display_event display_event);

void output(char* message);

/* io.c */
char* cursor_event_path(void);
void make_member_group_hex_dicts(struct charpcharp_llist** member_group_dict, struct charpcharp_llist** group_hex_dict);
void make_dicts(void);
void io_init(void);
void event_to_file(struct event event);
struct event file_to_event(char* file);
struct charp_llist* get_events_between(struct tm earlier_bound_tm, struct tm later_bound_tm);
struct charint_llist* get_color_dict(void);

/* utils.c */
char** split_args(char* string);
struct tm end_of_day(struct tm tm);
struct tm start_of_day(struct tm tm);
char* event_duration(int start_time, int end_time);
struct tm tm_add_interval(struct tm tm, int years, int months, int days);
int string_to_time(char* s);

void push_charp_llist(char* name, struct charp_llist** list);
void push_charpcharp_llist(char* content_1, char* content_2, struct charpcharp_llist** list);
void push_charpint_llist(char* c_content, int i_content, struct charpint_llist** list);
void push_eventp_llist(struct event event, struct eventp_llist** list);
void push_display_eventp_llist(struct display_event display_event, struct display_eventp_llist** list);

void free_charp_llist(struct charp_llist* list, bool malloced);
void free_charpcharp_llist(struct charpcharp_llist* list, bool malloced_1, bool malloced_2);
void free_charpint_llist(struct charpint_llist* list, bool malloced);
void free_eventp_llkist(struct eventp_llist* list);
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
void dump_charpp(char** charpp);
void print_cursor(int y, int x);

