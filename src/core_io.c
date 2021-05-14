#include "colormyday.h"

char* cmddata_path;
char* cmdconfig_path;
char* cmdgroups_path;

/*******************************************************************************
* EVENT READ/WRITE *************************************************************
********************************************************************************/
void 
event_to_file(
	struct event* event,
	struct error** err
) {
	(void)(err); // satisfy unused parameter for now ; do error handling later
	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "event");
	xmlDocSetRootElement(doc, root);
	xmlNewChild(root, NULL, BAD_CAST "title", BAD_CAST event->name);
	xmlNewChild(root, NULL, BAD_CAST "note", BAD_CAST event->note);
	char* new_file;
	asprintf(&new_file, "%s/%d", cmddata_path, event->start_time);
	if (event->end_time != TILL_NOW) {
		remove(new_file);
		asprintf(&new_file, "%s/%d-%d", cmddata_path, event->start_time, event->end_time);
	}
	xmlSaveFileEnc(new_file, doc, "UTF-8");
	xmlFreeDoc(doc);
}

struct event*
file_to_event(
	char* file
) {
	xmlDocPtr doc = xmlReadFile(file, NULL, 256);
	xmlNodePtr node = xmlDocGetRootElement(doc);

	/* What happens here? */
	char* file_dup = strdup(file);
	char* file_name = NULL;
	char* file_name_temp = strtok(file_dup, "/");
	while (file_name_temp != NULL) {
		file_name = file_name_temp;
		file_name_temp = strtok(NULL, "/");
	}
	
	/* Create event */
	struct event* event = malloc_event();
	char* time = strtok(file_name, "-");
	event->start_time = atoi(time);
	time = strtok(NULL, "-");
	if (time != NULL) {
		event->end_time = atoi(time);
	} else {
		event->end_time = TILL_NOW;
	}
	node = node->xmlChildrenNode;
	event->name = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	node = node->next;
	event->note = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);

	xmlFreeDoc(doc);
	free(file_dup);
	return event;
}

/********************************************************************************
* EVENT INFORMATION *************************************************************
*********************************************************************************/
static
int
filter_out_hidden(
	const struct dirent* entry
) {
	return (entry->d_name)[0] != '.';
}

char*
last_event_path(
	struct error** err
) {
	struct dirent** namelist;
	char* ret = NULL;
	int i = 0;
	char* file = NULL;
	int d = scandir(cmddata_path, &namelist, filter_out_hidden, alphasort);
	while (d--) {
		if (i == 0) {
			file = strdup(namelist[d]->d_name);
		}
		free(namelist[d]);
		i += 1;
	}
	free(namelist);
	if (file) {
		asprintf(&ret, "%s/%s", cmddata_path, file);
		free(file);
	} else {
		init_error(err, EMPTY_DATADIR, FATAL, NULL);
		return NULL;
	}
	return ret;
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
static
void
data_io_init(
	char* data_path
) {
	if (data_path) {
		asprintf(&cmddata_path, "%s", data_path);
	} else if (getenv("XDG_DATA_HOME")) {
		char* xdg_path = getenv("XDG_DATA_HOME");
		/* really should check ending slash before doing this */
		asprintf(&cmddata_path, "%s/colormyday", xdg_path);
	} else {
		char* home_path = getenv("HOME");
		char* cmd_path = NULL;
		/* really should check ending slash before doing this */
		asprintf(&cmd_path, "%s/.colormyday", home_path);
		asprintf(&cmddata_path, "%s/.colormyday/data", home_path);
		struct stat st = {0};
		if (stat(cmd_path, &st) == -1) {
			mkdir(cmd_path, 0700);
		}
		free(cmd_path);
	}
	struct stat st = {0};
	if (stat(cmddata_path, &st) == -1) {
		mkdir(cmddata_path, 0700);
	}
}

static
void
config_io_init(
	char* config_path
) {
	/* This is a dir */
	if (config_path) {
		asprintf(&cmdconfig_path, "%s", config_path);
	} else if (strcmp(getenv("XDG_CONFIG_HOME"), "") != 0) {
		char* xdg_path = getenv("XDG_CONFIG_HOME");
		asprintf(&cmdconfig_path, "%s/colormyday", xdg_path);
	} else {
		char* home_path = getenv("HOME");
		asprintf(&cmdconfig_path, "%s/.colormyday", home_path);
	}
	struct stat st = {0};
	if (stat(cmdconfig_path, &st) == -1) {
		mkdir(cmdconfig_path, 0700);
	}
}

static
void
groups_io_init(
	void
) {
	asprintf(&cmdgroups_path, "%s/groups", cmdconfig_path);
	if (access(cmdgroups_path, F_OK) == -1) {
		FILE* f;
		f = fopen(cmdgroups_path, "w");
		fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<groups>\n\t<group>\n\t\t<title>DEFAULT</title>\n\t\t<color>007FFF</color>\n\t\t<members>\n\t\t\t<event>DEFAULT</event>\n\t\t</members>\n\t</group>\n</groups>", f);
		fclose(f);
	}
}

void 
io_init(
	char* data_path,
	char* config_path,
	struct error** err
) {
	(void)(err); // satisfy unused parameter for now ; do error handling later
	data_io_init(data_path);
	config_io_init(config_path);
	groups_io_init();
}

