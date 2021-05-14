#include "colormyday.h"

int
main(
	int argc, 
	char* argv[]
) {
	/* Make options struct */
	setlocale(LC_ALL, "");
	struct error* err = NULL;
	struct options* opts = malloc_options();
	fill_options(opts, argc, argv, &err);
	if (err) {
		free_options(opts);
		catch_err_script(err);
	}

	io_init(opts->data_path, opts->config_path, &err);
	if (err) {
		free_options(opts);
		catch_err_script(err);
	}

	/* Start curses or script mode */
	if (opts->action == CURSES) {
		free_options(opts);
		main_gui(&err);
	} else {
		main_script(opts, &err);
	}
	catch_err_script(err);
	return 0;
}
