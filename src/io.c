#include "colormyday.h"

char* cmddata_path = NULL;
char* cmdconfig_path = NULL;
char* cmdgroups_path = NULL;

void 
make_member_group_hex_dicts(
	struct stringstring_llist** member_group_dict, 
	struct stringstring_llist** group_hex_dict
) {
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlReadFile(cmdgroups_path, NULL, 256);
	node = xmlDocGetRootElement(doc);

	*member_group_dict = NULL;
	*group_hex_dict = NULL;

	int i = 50;
	char *name, *hex, *member;
	xmlNodePtr sub_group;
	xmlNodePtr group = node->xmlChildrenNode;
	while(group && i < 256) {
		sub_group = group->xmlChildrenNode;
		name = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);
		
		sub_group = sub_group->next;
		hex = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);
		
		sub_group = sub_group->next->xmlChildrenNode;

		push_stringstring_llist(name, hex, group_hex_dict);

		while (sub_group) {
			member = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);

			push_stringstring_llist(member, name, member_group_dict);
			
			sub_group = sub_group->next;
		}

		i += 1;
		group = group->next;
	}
	
	xmlFreeDoc(doc);
}

/*
 * EVENT FUNCTIONS
 */
/*
 * Function: get_events_between
 *
 * In:
 *  earlier_bound_tm: the tm corresponding to the first second of the
 *  period in question
 *  later_bound_tm: the tm corresponding to the last second of the
 *  period in question
 *
 * Out:
 *  A linked list of strings of full paths to event files within the
 *  given period. This is sorted from most recent to oldest.
 *
 * This function collects all event files within a certain period,
 * meaning all events which occurred even in part within the period.
 *
 */
struct string_llist* 
get_events_between(
	struct tm earlier_bound_tm, 
	struct tm later_bound_tm
) {
	struct dirent** namelist;
	struct string_llist* ret = NULL;

	int earlier_bound, later_bound;

	if (earlier_bound_tm.tm_year != 1970) {
		earlier_bound = mktime(&earlier_bound_tm);

	} else {
		earlier_bound = -1;

	}

	if (later_bound_tm.tm_year != 1970) {
		later_bound = mktime(&later_bound_tm);

	} else {
		later_bound = -1;

	}

	char* file;
	char* file_temp;
	char* file_tempp;
	char* start_time;
	bool found = false;
	bool done = false;
	int d = scandir(cmddata_path, &namelist, NULL, alphasort);
	while (d--) {
		file = namelist[d]->d_name;
		file_temp = strdup(file);
		file_tempp = file_temp;

		if (!done) {
			if ((strcmp(file, ".") == 0) || (strcmp(file, "..") == 0)) {
				free(namelist[d]);
				free(file_tempp);
				continue;

			}

			start_time = strsep(&file_temp, "-");
			if (earlier_bound < atoi(start_time) && (atoi(start_time) < later_bound || later_bound == -1)) {
				asprintf(&file, "%s%s%s", cmddata_path, "/", file);
				push_string_llist(file, &ret);
				found = true;

			} else {
				if (found) {
					if (earlier_bound < atoi(strsep(&file_temp, "-"))) {
						asprintf(&file, "%s/%s", cmddata_path, file);
						push_string_llist(file, &ret);

					}

					done = true;

				}
			}
		}

		free(namelist[d]);
		free(file_tempp);

	}

	free(namelist);

	return ret;

}

/*
 * Function: event_to_file
 *
 * In:
 *  event: an event struct to be written to an event file
 *
 * This function writes to the event file corresponding to an event.
 * If one does not exist, it is created.
 *
 */
void 
event_to_file(
	struct event event
) {
	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "event");

	xmlDocSetRootElement(doc, root);

	xmlNewChild(root, NULL, BAD_CAST "title", BAD_CAST event.name);
	xmlNewChild(root, NULL, BAD_CAST "note", BAD_CAST event.note);

	char* new_file;
	asprintf(&new_file, "%s/%d", cmddata_path, event.start_time);
	if (event.end_time != -1) {
		remove(new_file);
		asprintf(&new_file, "%s/%d-%d", cmddata_path, event.start_time, event.end_time);

	}

	xmlSaveFileEnc(new_file, doc, "UTF-8");
	xmlFreeDoc(doc);

}

/*
 * Function: file_to_event
 *
 * In:
 *  file: a full path to an event file
 *
 * Out:
 *  An event struct corresponding to the file.
 *
 * This function turns an event file into an event struct.
 * 
 */
struct event 
file_to_event(
	char* file
) {
	struct event e;
	
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlReadFile(file, NULL, 256);
	node = xmlDocGetRootElement(doc);

	char* file_dup;
	asprintf(&file_dup, "%s", file);

	char* file_name = NULL;
	char* file_name_temp = strtok(file_dup, "/");
	while (file_name_temp != NULL) {
		file_name = file_name_temp;
		file_name_temp = strtok(NULL, "/");

	}

	char* time = strtok(file_name, "-");
	e.start_time = atoi(time);

	time = strtok(NULL, "-");
	if (time != NULL) {
		e.end_time = atoi(time);

	} else {
		e.end_time = -1;

	}

	node = node->xmlChildrenNode;
	e.name = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);

	node = node->next;
	e.note = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);

	xmlFreeDoc(doc);
	
	free(file_dup);

	return e;

}

char* 
cursor_event_path(
	void
) {
	char* ret = NULL;
	
	if (cursor_event.event.end_time == -1) {
		asprintf(&ret, "%s/%d", cmddata_path, cursor_event.event.start_time);

	} else {
		asprintf(&ret, "%s/%d-%d", cmddata_path, cursor_event.event.start_time, cursor_event.event.end_time);

	}

	return ret;

}

int
filter_out_hidden(
	const struct dirent* entry
) {
	return (entry->d_name)[0] != '.';

}

/*
 * Function: last_event_path
 *
 * Out:
 *  The full path to the last recorded event file.
 *
 * This function gets the path to the last (most recent) event file.
 *
 */
char*
last_event_path(
	void
) {
	struct dirent** namelist;
	char* ret = NULL;

	int i = 0;
	char* file = NULL;
	int d = scandir(cmddata_path, &namelist, filter_out_hidden, alphasort);

	while (d--) {
		if (i == 0) {
			file = namelist[d]->d_name;
		}
		free(namelist[d]);

		i += 1;

	}

	free(namelist);

	if (file != NULL) {
		asprintf(&ret, "%s/%s", cmddata_path, file);

	}

	return ret;

}

/*
 * INITIALIZERS
 */
void 
io_init(
	char* data_path,
	char* config_path
) {
	/* check/create db directory in home */	
	if (data_path) {
		asprintf(&cmddata_path, "%s", data_path);

	} else {
		char* xdg_path = getenv("XDG_DATA_HOME");
		asprintf(&cmddata_path, "%s%s", xdg_path, "/colormyday");

	}

	if (config_path) {
		asprintf(&cmdconfig_path, "%s", config_path);

	} else {
		char* xdg_path = getenv("XDG_CONFIG_HOME");
		asprintf(&cmdconfig_path, "%s%s", xdg_path, "/colormyday");

	}

	asprintf(&cmdgroups_path, "%s%s", cmdconfig_path, "/groups");

	struct stat st = {0};
	if (stat(cmddata_path, &st) == -1) {
		mkdir(cmddata_path, 0700);

	}

	if (stat(cmdconfig_path, &st) == -1) {
		mkdir(cmdconfig_path, 0700);

	}

	if (access(cmdgroups_path, F_OK) == -1) {
		FILE* f;
		f = fopen(cmdgroups_path, "w");
		fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<groups>\n\t<group>\n\t\t<title>DEFAULT</title>\n\t\t<color>007FFF</color>\n\t\t<members>\n\t\t\t<event>DEFAULT</event>\n\t\t</members>\n\t</group>\n</groups>", f);
		fclose(f);

	}
}

