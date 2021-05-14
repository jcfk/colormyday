#include "colormyday.h"

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
void
free_options(
	struct options* opts
) {
	free(opts);
}

/*********************************************************************
* FILLER *************************************************************
**********************************************************************/
void
fill_options(
	struct options* opts,
	int argc,
	char* argv[],
	struct error** err
) {
	/* Initialize opts */
	opts->begin_name = NULL;
	opts->late_time = NULL;
	opts->action = CURSES;

	/* Parse --options */
	opterr = 0;
	int flag;
	while (true) {
		static struct option long_options[] = {
			{"datadir", required_argument, NULL, 'd'},
			{"confdir", required_argument, NULL, 'c'},
			{"late", required_argument, NULL, 'l'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		flag = getopt_long(argc, argv, "", long_options, &option_index);
		if (flag == -1) {
			break;
		}
		/* consider using opterr in order to error on unknown option */
		switch (flag) {
			case 'd':
				opts->data_path = optarg;
				break;
			case 'c':
				opts->config_path = optarg;
				break;
			case 'l':
				opts->late_time = optarg;
				break;
			case '?':
				init_error(err, UNKNOWN_OPTION, NONFATAL, "%s", argv[optind-1]);
				return;
		}
	}

	/* Parse action */
	argc = argc - optind;
	argv = argv + optind;
	if (argc < 1) {
		return;
	}
	char* action_string = argv[0];
	if STREQ(action_string, "begin") {
		opts->action = BEGIN;
		if (argc < 2) {
			init_error(err, MISSING_EVENT_NAME, NONFATAL,
				"\"begin\" must be followed by event name. See manpage.");
			return;
		}
		char* name_string = argv[1];
		opts->begin_name = name_string;
	} else if STREQ(action_string, "end") {
		opts->action = END;
	} else if STREQ(action_string, "show") {
		opts->action = SHOW;
		opts->show_count = 20;
		if (argc > 1) {
			char* show_count_string = argv[1];
			char* temp;
			int show_count = strtol(show_count_string, &temp, 10);
			if (show_count == 0) {
				init_error(err, BAD_SHOW_COUNT, NONFATAL, NULL);
				return;
			}
			opts->show_count = show_count;
		}
	} else {
		init_error(err, UNKNOWN_ACTION, NONFATAL, "%s", action_string);
		return;
	}
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
struct options*
malloc_options(
	void
) {
	struct options* opts = malloc(sizeof(struct options));
	opts->data_path = NULL;
	opts->config_path = NULL;
	opts->late_time = NULL;
	opts->begin_name = NULL;
	return opts;
}
