#include "colormyday.h"

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
static
void
free_script(
	void
) {
	free(cmddata_path);
	free(cmdconfig_path);
	free(cmdgroups_path);
}

void
exit_from_script(
	int status
) {
	free_script();
	exit(status);
}

/*********************************************************************
 * ACTIONS ***********************************************************
 *********************************************************************/
/* static */
/* void */
/* main_show( */
/* 	int show_count */
/* ) { */
/* 	struct eventp_llist* last_events = last_few_events(show_count); */
	
/* } */

static
void
main_end(
	char* late_time,
	struct error** err
) {
	/* Parse late time */
	int late_time_seconds = time(NULL);
	if (late_time) {
		late_time_seconds = string_to_time(late_time, err);
		if ERRP return;
		validate_late_time(late_time_seconds, time(NULL), err);
		if ERRP return;
	}

	/* End current event */
	struct event* last = last_event(err);
	if ERRP return;
	if (last->end_time != TILL_NOW) {
		init_error(err, ALREADY_ENDED_EVENT, FATAL, 
			"Last event \"%s\" already ended.", last->name);
		return;
	}
	end_current_event_core(late_time_seconds, err);
	if ERRP return;
	printf("Ending \"%s\" with end_time: %s\n", last->name, late_time);
}

static
void
main_begin(
	char* name,
	char* late_time,
	struct error** err
) {
	/* Parse late time */
	int late_time_seconds = time(NULL);
	if (late_time) {
		late_time_seconds = string_to_time(late_time, err);
		if ERRP return;
		validate_late_time(late_time_seconds, time(NULL), err);
		if ERRP return;
	}

	/* End current event */
	struct event* last = last_event(err);
	if ERRP return;
	if (last->end_time == TILL_NOW) {
		end_current_event_core(late_time_seconds, err);
		if ERRP return;
	}

	/* Begin new event */
	begin_event_core(name, late_time_seconds, err);
	if ERRP return;
	printf("Starting \"%s\" with begin_time: %s\n", name, late_time);
}

/*********************************************************************
 * INITIALIZER *******************************************************
 *********************************************************************/
void
main_script(
	struct options* opts,
	struct error** err
) {
	switch (opts->action) {
		case BEGIN:
			main_begin(opts->begin_name, opts->late_time, err);
			break;
		case END:
			main_end(opts->late_time, err);
			break;
		/* case SHOW: */
		/* 	main_show(opts->show_count); */
		/* 	break; */
		default:
			break;
	}
	free_options(opts);
	if ERRP return;
	exit_from_script(EXIT_SUCCESS);
}

