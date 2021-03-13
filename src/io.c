#include "colormyday.h"

char* cmd_path = NULL;
char* cmddb_path = NULL;
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
	int d = scandir(cmddb_path, &namelist, NULL, alphasort);
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
				asprintf(&file, "%s%s%s", cmddb_path, "/", file);
				push_string_llist(file, &ret);
				found = true;

			} else {
				if (found) {
					if (earlier_bound < atoi(strsep(&file_temp, "-"))) {
						asprintf(&file, "%s/%s", cmddb_path, file);
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
	asprintf(&new_file, "%s/%d", cmddb_path, event.start_time);
	if (event.end_time != -1) {
		remove(new_file);
		asprintf(&new_file, "%s/%d-%d", cmddb_path, event.start_time, event.end_time);

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
		asprintf(&ret, "%s/%d", cmddb_path, cursor_event.event.start_time);

	} else {
		asprintf(&ret, "%s/%d-%d", cmddb_path, cursor_event.event.start_time, cursor_event.event.end_time);

	}

	return ret;

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
	char* ret;

	char* file;
	int d = scandir(cmddb_path, &namelist, NULL, alphasort);
	d--;
	
	file = namelist[d]->d_name;

	while (d--) {
		free(namelist[d]);

	}

	free(namelist);

	asprintf(&ret, "%s/%s", cmddb_path, file);

	return ret;

}

/*
 * INITIALIZERS
 */
void 
io_init(
	char* path
) {
	/* check/create db directory in home */	
	if (path) {
		asprintf(&cmd_path, "%s", path);

	} else {
		char* home_path = getenv("HOME");
		asprintf(&cmd_path, "%s%s%s", home_path, "/", CMD_DIRECTORY_PATH);

	}

	asprintf(&cmddb_path, "%s%s", cmd_path, "/data");
	asprintf(&cmdgroups_path, "%s%s", cmd_path, "/groups");

	struct stat st = {0};
	if (stat(cmd_path, &st) == -1) {
		mkdir(cmd_path, 0700);

	}
	
	if (stat(cmddb_path, &st) == -1) {
		mkdir(cmddb_path, 0700);

	}

	if (access(cmdgroups_path, F_OK) == -1) {
		FILE* f;
		f = fopen(cmdgroups_path, "w");
		fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<groups>\n\t<group>\n\t\t<title>DEFAULT</title>\n\t\t<color>007FFF</color>\n\t\t<members>\n\t\t\t<event>DEFAULT</event>\n\t\t</members>\n\t</group>\n</groups>", f);
		fclose(f);

	}
}

