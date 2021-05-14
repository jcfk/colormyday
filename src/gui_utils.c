#include "colormyday.h"
#include "gui.h"

/***************************************************************************
* ARGV CREATOR *************************************************************
****************************************************************************/
void
free_argv(
	int argc,
	char** argv
) {
	int i = 0;
	char** temp = argv;
	while (i < argc) { // fix
		free(temp[i]);
		i = i + 1;
	}
	free(argv);
}

int
fill_argv(
	char*** argv,
	char* string
) {
	int i = 0;
	int space_count = 0;
	while (string[i]) {
		if (string[i] == ' ') {
			space_count = space_count + 1;
		}
		i = i + 1;
	}

	*argv = malloc(sizeof(char*)*(space_count+2));
	char* dummy = NULL;
	asprintf(&dummy, "dummy");
	(*argv)[0] = dummy; //dummy 0th arg to satisfy getopt
	i = 0;
	int len = 0;
	int argc = 2;
	char* arg = malloc((sizeof(char)*strlen(string))+1);
	bool dquoted = false, squoted = false;
	while (string[i]) {
		if (string[i] == '\"' && !squoted) {
			dquoted = !dquoted;
		} else if (string[i] == '\'' && !dquoted) {
			squoted = !squoted;
		} else if (string[i] == ' ' && !squoted && !dquoted) {
			len = 0;
			(*argv)[argc-1] = arg;
			arg = malloc((sizeof(char)*strlen(string))+1);
			arg[0] = '\0';
			argc = argc + 1;
		} else {
			arg[len] = string[i];
			arg[len+1] = '\0';
			len = len + 1;
		}
		i = i + 1;
	}
	(*argv)[argc-1] = arg;
	return argc;
}

/*******************************************************************
* TIME *************************************************************
********************************************************************/
char*
time_to_dsp_string(
	int time
) {
	char* str = malloc(sizeof(char)*50);
	struct tm* temp_tm = malloc(sizeof(struct tm));
	time_t temp_t = time;
	localtime_r(&temp_t, temp_tm); // why?
	strftime(str, 50, "%a, %H:%M:%S", temp_tm);
	free(temp_tm);
	return str; // free str?
}

struct tm 
tm_add_interval(
	struct tm tm, 
	int years, 
	int months, 
	int days
) {
	struct tm temp = tm;
	temp.tm_year = temp.tm_year + years;
	temp.tm_mon = temp.tm_mon + months;
	temp.tm_mday = temp.tm_mday + days;
	temp.tm_isdst = -1;
	time_t temp_t = mktime(&temp);
	return *localtime(&temp_t);
}

struct tm 
end_of_day(
	struct tm tm
) {
	struct tm temp = tm;
	temp.tm_hour = 23;
	temp.tm_min = 59;
	temp.tm_sec = 59;
	return temp;
}

struct tm 
start_of_day(
	struct tm tm
) {
	struct tm temp = tm;
	temp.tm_hour = 0;
	temp.tm_min = 0;
	temp.tm_sec = 0;
	return temp;
}

char* 
event_duration(
	int start_time, 
	int end_time
) {
	char* ret;
	int duration = end_time - start_time;
	int h = duration / 3600;
	int m = (duration % 3600)/60;
	int s = (duration % 3600) % 60;	
	asprintf(&ret, "%d:%d:%d", h, m, s);
	return ret;
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
struct dsp_event*
malloc_dsp_event(
	void
) {
	struct event* event = malloc_event();
	struct dsp_event* dsp_event = malloc(sizeof(struct dsp_event));
	dsp_event->event = event;
	return dsp_event;
}
