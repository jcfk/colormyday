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

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <ncurses.h>

#define CMD_DIRECTORY_NAME ".colormyday"
#define CMD_DB_NAME "data"

/* global instances */
pthread_mutex_t display_access, variable_access;
struct charpcharp_llist* member_group_dict;
struct charpcharp_llist* group_hex_dict;
struct charpint_llist* group_color_dict;
struct eventp_llist* current_events;
struct event current_event;
struct event selected_event;
char* current_event_path;
int current_time;
int earlier_bound_day, later_bound_day;
bool cursor_ticking;

/* tick.c */
void* tick_init(void* arg);

/* input.c */
void key_up();

/* data.c */
void data_init();
int make_color(char* code);
void make_group_color_dict();
void set_end(struct event event);

/* display.c */
extern WINDOW* top_data;
extern WINDOW* rainbow;
extern WINDOW* bottom_data;
extern WINDOW* controls;
extern int term_h, term_w;
extern int top_data_x, top_data_y, top_data_h, top_data_w;
extern int rainbow_x, rainbow_y, rainbow_h, rainbow_w;
extern int bottom_data_x, bottom_data_y, bottom_data_h, bottom_data_w;
extern int controls_x, controls_y, controls_h, controls_w;
extern int color_width;
extern double slot_duration;

extern bool end_visible;
extern int cursor[2];

char* get_command();
void display_duration(struct event event);
void display_event(struct event event);
void display_end_event(struct event event);
void cursor_tick();
void cursor_init();
void windows_init();
void display_init();

/* io.c */
extern char* cmd_path;
extern char* cmddb_path;

void make_member_group_hex_dicts();
void make_dicts();
void io_init();
void begin_event(char* name);
struct event end_current_event();
struct event file_to_event(char* file);
struct charp_llist* get_events_between(int earlier_bound, int later_bound);
struct charint_llist* get_color_dict();

/* utils.c */
extern int no_of_colors;

int end_of_day(struct tm* tm);
int start_of_day(struct tm* tm);
struct tm* time_to_tm_local(int time);
void push_charp_llist(char* name, struct charp_llist** list);
void push_charpcharp_llist(char* content_1, char* content_2, struct charpcharp_llist** list);
void push_charpint_llist(char* c_content, int i_content, struct charpint_llist** list);
void push_eventp_llist(struct event event, struct eventp_llist** list);
int charpint_dict(struct charpint_llist* list, char* c);
char* charpcharp_dict(struct charpcharp_llist* list, char* c);
void dump_charp_llist(struct charp_llist* list);
void dump_charpcharp_llist(struct charpcharp_llist* list);
void dump_charpint_llist(struct charpint_llist* list);
void dump_event(struct event event);
void dump_eventp_llist(struct eventp_llist* list);

/* structs */
struct event {
	int start_time, end_time;
	char* name;
	char* note;
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

enum cursor_movement {
	LEFT,
	DOWN,
	UP,
	RIGHT
};
