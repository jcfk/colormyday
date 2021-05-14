#include <ncurses.h>

#define TEXT_EDITOR "nano"
#define TOP_DATA_H 3
#define BOTTOM_DATA_H 7
#define CONTROLS_H 1
#define MIN_TERMINAL_H 18
#define MIN_TERMINAL_W 105

/********************************************************************
* ENUMS *************************************************************
*********************************************************************/
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
	C_B,
	C_NOTHIN
};

enum rainbow_scroll {
	R_UP,
	R_DOWN
};

enum gui_action {
	G_EDIT,
	G_BEGIN,
	G_END,
	G_QUIT
};

/**********************************************************************
* STRUCTS *************************************************************
***********************************************************************/
struct gui_options {
	enum gui_action action;
	char* late_time;
	char* begin_name;
};

/**********************************************************************
* GLOBALS *************************************************************
***********************************************************************/
extern pthread_mutex_t            globals_display;
extern struct tm                  earlier_bound_day;
extern struct tm                  later_bound_day;
extern struct dsp_eventp_llist*   current_events;
extern struct dsp_event*          current_event;
extern struct dsp_event*          cursor_event;
extern struct stringstring_llist* member_group_dict;
extern struct stringstring_llist* group_hex_dict;
extern struct stringint_llist*    group_color_dict;
extern bool                       cursor_ticking;
extern bool                       thread_exit;
extern WINDOW*                    top_data;
extern WINDOW*                    rainbow;
extern WINDOW*                    bottom_data;
extern WINDOW*                    controls;
extern int                        current_time;


/************************************************************************
* FUNCTIONS *************************************************************
*************************************************************************/
void                   cursor_move(                 enum cursor_movement movement,
                                                    struct error** err);

void                   resize(               struct error** err);

void                   input_handle(                int key,
                                                    struct error** err);

void                   reload_current_events(       struct error** err);

void                   gui_data_init(            int rainbow_h,
                                                    struct error** err);

void                   display_error(               char* string);

void                   display_events(              void);

void                   display_tick(                struct error** err);

void                   curses_start(                void);

void                   display_note(                struct dsp_event* dsp_event);

char*                  cursor_event_path(           struct error** err);

void                   push_stringint_llist(        char* c_content,
                                                    int i_content,
                                                    struct stringint_llist** list);

void                   push_eventp_llist(           struct event* event,
                                                    struct eventp_llist** list);

void                   push_dsp_eventp_llist(       struct dsp_event* dsp_event,
                                                    struct dsp_eventp_llist** list);

void                   free_stringint_llist(        struct stringint_llist* list,
                                                    bool malloced);

void                   free_dsp_eventp_llist(       struct dsp_eventp_llist* list);

int                    stringint_dict(              struct stringint_llist* list,
                                                    char* c);

struct tm              tm_add_interval(             struct tm tm,
                                                    int years,
                                                    int months,
                                                    int days);

struct tm              end_of_day(                  struct tm tm);

struct tm              start_of_day(                struct tm tm);

char*                  event_duration(              int start_time,
                                                    int end_time);

void                   make_member_group_hex_dicts( struct error** err);

struct string_llist*   get_events_between(          struct tm earlier_bound_tm,
                                                    struct tm later_bound_tm);

char*                  get_command(                 void);

void                   end_current_event_gui(    int late_time,
                                                    struct error** err);

void                   begin_event_gui(          char* name,
                                                    int late_time,
                                                    struct error** err);

struct dsp_event*      malloc_dsp_event(            void);

int                    fill_argv(                   char*** argv,
                                                    char* string);

struct gui_options* malloc_gui_options(             void);

void                   fill_gui_options(         struct gui_options* opts,
                                                    int argc,
                                                    char** argv,
                                                    struct error** err);

int                    curses_windows_init(         struct error** err);

void                   end_curses(                  void);

char*                  time_to_dsp_string(          int time);

void                   curses_tick(                 void);

void                   free_argv(                   int argc,
                                                    char** argv);

void                   free_gui_options(         struct gui_options* opts);


