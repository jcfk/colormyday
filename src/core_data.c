#include "colormyday.h"

/*************************************************************************
* EVENT DATA *************************************************************
**************************************************************************/
struct event*
last_event(
	struct error** err
) {
	char* path = last_event_path(err);
	if (ERRP && (*err)->status == EMPTY_DATADIR) {
		clear_error(err);
		return NULL;
	}
	if ERRP return NULL;
	struct event* event = file_to_event(path);
	free(path);
	return event;
}

void
validate_late_time(
	int late_time,
	int current_time,
	struct error** err
) {
	struct event* last = last_event(err);
	if ERRP return;
	int last_event_beginning = last->start_time;
	if (late_time <= last_event_beginning) {
		init_error(err, TOO_EARLY_TIMESTAMP, NONFATAL, 
			"Can't begin event before the most recent: '%s'", late_time);
		return;
	} else if (late_time > current_time) {
		init_error(err, FUTURE_TIMESTAMP, NONFATAL, 
			"Can't begin event in the future: '%s'", late_time);
		return;
	}
}

/**********************************************************************
* ACTIONS *************************************************************
***********************************************************************/
void
end_current_event_core(
	int late_time,
	struct error** err
) {
	struct event* last = last_event(err);
	if ERRP return;
	if (last->end_time != TILL_NOW) {
		init_error(err, ALREADY_ENDED_EVENT, NONFATAL, NULL);
		return;
	}
	last->end_time = late_time;
	event_to_file(last, err);
	if ERRP return;
}

void
begin_event_core(
	char* name, 
	int late_time,
	struct error** err
) {
	struct event temp = (struct event) { // no need to free, see?
		.start_time = late_time,
		.end_time = TILL_NOW,
		.name = name,
		.note = "-1"
	};
	event_to_file(&temp, err);
	if ERRP return;
}

