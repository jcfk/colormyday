#include "colormyday.h"

void tick() {
	for(;;) {
		sleep(1);

		pthread_mutex_lock(&variable_access);
		pthread_mutex_lock(&display_access);

		current_time = time(NULL);

		if (current_event.event.name != NULL) {
			display_duration(current_event);
			display_end_event(current_event);
		}

		display_tick();

		pthread_mutex_unlock(&display_access);
		pthread_mutex_unlock(&variable_access);
	}
}

void* tick_init(void* arg) {
	tick();

	return 0;
}
