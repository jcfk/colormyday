#include "colormyday.h"

void 
dump_opts(
	struct options* opts
) {
	printf("action: %d\ndata_path: %s\nconfig_path: %s\nlate_time: %s\nbegin_name: %s\nshow_count: %d\n", 
		opts->action,
		opts->data_path,
		opts->config_path,
		opts->late_time,
		opts->begin_name,
		opts->show_count);
}


