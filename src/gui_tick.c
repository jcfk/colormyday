#include "colormyday.h"
#include "gui.h"

pthread_mutex_t globals_display;

static
void
catch_err_tick(
	struct error* err
) {
	if (err) {
		// What happens here?
	}
}

/*******************************************************************
* MAIN *************************************************************
********************************************************************/
static 
void 
tick(
	void
) {
	while (1) {
		sleep(5);
		pthread_mutex_lock(&globals_display);
		current_time = time(NULL);
		struct error* err = NULL;
		display_tick(&err);
		catch_err_tick(err);
		pthread_mutex_unlock(&globals_display);
	}
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
static
void* 
tick_init(
	void* arg
) {
	(void)arg;
	tick();
	return NULL;
}

void
curses_tick(
	void
) {
	pthread_mutex_init(&globals_display, NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t tick_thread;
	pthread_create(&tick_thread, &attr, tick_init, NULL);
}

