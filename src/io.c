#include "colormyday.h"

char* cmd_path = NULL;
char* cmddb_path = NULL;
char* cmdgroups_path = NULL;

void 
make_member_group_hex_dicts(
	struct charpcharp_llist** member_group_dict, 
	struct charpcharp_llist** group_hex_dict
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

		push_charpcharp_llist(name, hex, group_hex_dict);

		while (sub_group) {
			member = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);

			push_charpcharp_llist(member, name, member_group_dict);
			
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
struct charp_llist* 
get_events_between(
	struct tm earlier_bound_tm, 
	struct tm later_bound_tm
) {
	struct dirent** namelist;
	struct charp_llist* ret = NULL;

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
				push_charp_llist(file, &ret);
				found = true;

			} else {
				if (found) {
					if (earlier_bound < atoi(strsep(&file_temp, "-"))) {
						asprintf(&file, "%s/%s", cmddb_path, file);
						push_charp_llist(file, &ret);

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

void event_to_file(struct event event) {
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

struct event file_to_event(char* file) {
	struct event e;
	
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlReadFile(file, NULL, 256);
	node = xmlDocGetRootElement(doc);

	char* file_dup;
	asprintf(&file_dup, "%s", file);

	char* file_name;
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

char* cursor_event_path() {
	char* ret = NULL;
	
	if (cursor_event.event.end_time == -1) {
		asprintf(&ret, "%s/%d", cmddb_path, cursor_event.event.start_time);

	} else {
		asprintf(&ret, "%s/%d-%d", cmddb_path, cursor_event.event.start_time, cursor_event.event.end_time);

	}

	return ret;

}

/*
struct event
last_event_io(
	void
) {
	

}
*/

/*
 * INITIALIZERS
 */
void io_init(char* path) {
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
		fclose(f);

	}
}

