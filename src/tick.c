#include "colormyday.h"

static void 
tick(
	void
) {
	for(;;) {
		sleep(5);

		pthread_mutex_lock(&variable_access);
		pthread_mutex_lock(&display_access);

		current_time = time(NULL);

		display_tick();

		pthread_mutex_unlock(&display_access);
		pthread_mutex_unlock(&variable_access);
	}
}

void* 
tick_init(
	void* arg
) {
	(void)arg;

	tick();

	return 0;
}
