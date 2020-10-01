#include "colormyday.h"

void tick() {
	for(;;) {
		sleep(1);

		pthread_mutex_lock(&variable_access);
		pthread_mutex_lock(&display_access);

		if (thread_exit) {
			pthread_exit(0);

		}

		current_time = time(NULL)-3600*0-60*0;

		display_tick();

		pthread_mutex_unlock(&display_access);
		pthread_mutex_unlock(&variable_access);
	}
}

void* tick_init(void* arg) {
	tick();

	return 0;
}
