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
#include <getopt.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libxml2/libxml/parser.h> // libxml2/libxml??
#include <libxml2/libxml/tree.h>

#define CMD_VERSION "COLORMYDAY v" VERSION
#define GENERATE_ERRPOR_STATUSES(FUNC) \
	FUNC(GENERIC_ERRPOR) \
	FUNC(UNKNOWN_COMMAND) \
	FUNC(MISSING_EVENT_NAME) \
	FUNC(BAD_TIMESTAMP) \
	FUNC(TOO_EARLY_TIMESTAMP) \
	FUNC(FUTURE_TIMESTAMP) \
	FUNC(ALREADY_ENDED_EVENT) \
	FUNC(EMPTY_DATADIR) \
	FUNC(CANNOT_OPEN_FILE) \
	FUNC(BAD_SHOW_COUNT) \
	FUNC(UNKNOWN_OPTION) \
	FUNC(UNKNOWN_ACTION) \
	FUNC(MISSING_ARGUMENT) \
	FUNC(NO_TERMINAL_COLORS) \
	FUNC(TOO_SMALL_TERMINAL) \
	FUNC(BAD_HEX_COLOR_CODE) \
	FUNC(NO_EVENT_SELECTED) \
	FUNC(BAD_XML)
#define STRINGIZE(X) #X,
#define ENUMIZE(X) X,
#define ERRP (*err)
#define STREQ(a, b) (strcmp(a, b) == 0)
#define STRNEQ(a, b) (strcmp(a, b) != 0)
#define TILL_NOW -1

/********************************************************************
* ENUMS *************************************************************
*********************************************************************/
enum action {
	BEGIN,
	END,
	SHOW,
	CURSES
};

enum error_status {
	GENERATE_ERRPOR_STATUSES(ENUMIZE)
};

enum fatal_status {
	NONFATAL,
	FATAL
};

/**********************************************************************
* STRUCTS *************************************************************
***********************************************************************/
struct event {
	int start_time, end_time;
	char* name;
	char* note;
};

struct dsp_event {
	char* group;
	int color;
	struct event* event;
};

struct string_llist {
	char* content;
	struct string_llist* next;
};

struct stringint_llist {
	char* c_content;
	int i_content;
	struct stringint_llist* next;
};

struct stringstring_llist {
	char* content_1;
	char* content_2;
	struct stringstring_llist* next;
};

struct eventp_llist {
	struct event* event;
	struct eventp_llist* next;
};

struct dsp_eventp_llist {
	struct dsp_event* dsp_event;
	struct dsp_eventp_llist* next;
};

struct options {
	enum action action;
	char* data_path;
	char* config_path;
	char* late_time;
	char* begin_name;
	int show_count;
};

struct error {
	enum error_status status;
	bool fatal;
	char* message;
};

/**********************************************************************
* GLOBALS *************************************************************
***********************************************************************/
extern char* error_status_strings[];
extern char* cmddata_path;
extern char* cmdconfig_path;
extern char* cmdgroups_path;

/************************************************************************
* FUNCTIONS *************************************************************
*************************************************************************/
void            fill_options(            struct options* opts,
                                         int argc,
                                         char* argv[],
                                         struct error** err);

void            free_options(            struct options* opts);

void            main_gui(                struct error** err);

void            io_init(                 char* data_path,
                                         char* config_path,
                                         struct error** err);

int             string_to_time(          char* s,
                                         struct error** err);

void            push_string_llist(       char* name,
                                         struct string_llist** list);

void            push_stringstring_llist( char* content_1,
                                         char* content_2,
                                         struct stringstring_llist** list);

void            event_to_file(           struct event* event,
                                         struct error** err);

struct event*   file_to_event(           char* file);

char*           last_event_path(         struct error** err);

struct event*   last_event(              struct error** err);

void            free_string_llist(       struct string_llist* list,
                                         bool malloced);

void            free_stringstring_llist( struct stringstring_llist* list,
                                         bool malloced_1,
                                         bool malloced_2);

void            begin_event_core(        char* name,
                                         int late_time,
                                         struct error** err);

void            init_error(              struct error** ptr,
                                         enum error_status status,
                                         enum fatal_status fatal,
                                         char* msg_fmt,
                                         ...);

struct options* malloc_options(          void);

void            end_current_event_core(  int late_time,
                                         struct error** err);

void            begin_event_core(        char* name,
                                         int late_time,
                                         struct error** err);

struct event*   malloc_event(            void);

void            main_script(             struct options* opts,
                                         struct error** err);

void            dump_opts(               struct options* opts);

void            catch_err_script(        struct error* err);

void            exit_from_script(        int status);

void            add_error_msg(           struct error** ptr,
                                         char* msg_fmt,
                                         ...);

char*           error_status_to_string(  enum error_status status);

void            free_error(              struct error* err);

void            free_options(            struct options* opts);

char*           stringstring_dict(       struct stringstring_llist* list,
                                         char* c);

void            catch_err_gui(        struct error* err);

void            exit_from_gui(        int status);

bool            regex_match(             char* regex,
                                         char* str);

void            clear_error(             struct error** err);

void            validate_late_time(      int late_time,
                                         int current_time,
                                         struct error** err);


