#include "colormyday.h"
#include "gui.h"

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
void
free_gui_options(
	struct gui_options* opts
) {
	free(opts->begin_name); // have to free argv this time...
	free(opts->late_time);
	free(opts);
}

/*********************************************************************
* FILLER *************************************************************
**********************************************************************/
void
fill_gui_options(
	struct gui_options* opts,
	int argc,
	char** argv,
	struct error** err
) {
	/* Initialize opts */
	opts->begin_name = NULL;
	opts->late_time = NULL;

	/* Parse --options */
	optind = 1;
	opterr = 0;
	while (true) {
		static struct option long_options[] = {
			{"late", required_argument, NULL, 'l'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		int flag = getopt_long(argc, argv, "", long_options, &option_index);
		if (flag == -1) break;
		switch (flag) {
			case 'l':
				opts->late_time = strdup(optarg);
				break;
			case '?':
				init_error(err, UNKNOWN_OPTION, NONFATAL, "%s", argv[optind-1]);
				return;
		}
	}

	/* Parse action */
	argc = argc - optind;
	argv = argv + optind;
	if (argc == 0) return;
	/* opts->action = C_NONE; */
	char* action_string = argv[0];
	if STREQ(action_string, "begin") {
		opts->action = G_BEGIN;
		if (argc < 2) {
			init_error(err, MISSING_EVENT_NAME, NONFATAL,
				"\"begin\" must be followed by event name. See manpage.");
			return;
		}
		char* name_string = argv[1];
		opts->begin_name = strdup(name_string);
	} else if STREQ(action_string, "end") {
		opts->action = G_END;
	} else if STREQ(action_string, "edit") {
		opts->action = G_EDIT;
	} else if STREQ(action_string, "q") {
		opts->action = G_QUIT;
	} else {
		init_error(err, UNKNOWN_ACTION, NONFATAL, "%s", action_string);
		return;
	}
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
struct gui_options*
malloc_gui_options(
	void
) {
	struct gui_options* opts = malloc(sizeof(struct gui_options));
	return opts;
}

