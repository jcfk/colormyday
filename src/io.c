#include "colormyday.h"

char* cmd_path = NULL;
char* cmddb_path = NULL;
char* cmdgroups_path = NULL;
char* current_event_path;

void make_member_group_hex_dicts(struct charpcharp_llist** member_group_dict, struct charpcharp_llist** group_hex_dict) {
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
struct charp_llist* get_events_between(int earlier_bound, int later_bound) {
	struct dirent** namelist;
	struct charp_llist* ret = NULL;

	char* file;
	char* file_temp;
	char* start_time;
	bool found = false;
	int d = scandir(cmddb_path, &namelist, 0, alphasort);
	while (d--) {
		file = namelist[d]->d_name;
		asprintf(&file_temp, "%s", file);

		if ((strcmp(file, ".") == 0) || (strcmp(file, "..") == 0)) {
			break;
		}

		start_time = strsep(&file_temp, "-");
		if (earlier_bound < atoi(start_time) && (atoi(start_time) < later_bound || later_bound == -1)) {
			if (!found) {
				asprintf(&current_event_path, "%s/%s", cmddb_path, file);
			}

			asprintf(&file, "%s%s%s", cmddb_path, "/", file);
			push_charp_llist(file, &ret);
			found = true;
		} else {
			if (found) {
				if (earlier_bound < atoi(strsep(&file_temp, "-"))) {
					asprintf(&file, "%s/%s", cmddb_path, file);
					push_charp_llist(file, &ret);
				}
				break;
			}
		}
		free(namelist[d]);
	}

	free(namelist);

	return ret;
}

void event_to_file(struct event event) {
	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "event");

	xmlDocSetRootElement(doc, root);

	char* start_time;
	asprintf(&start_time, "%d", event.start_time);

	char* end_time;
	asprintf(&end_time, "%d", event.end_time);

	xmlNewChild(root, NULL, BAD_CAST "start", BAD_CAST start_time);
	xmlNewChild(root, NULL, BAD_CAST "end", BAD_CAST end_time);
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

	node = node->xmlChildrenNode;
	e.start_time = atoi((char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1));
	node = node->next;
	e.end_time = atoi((char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1));
	node = node->next;
	e.name = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	node = node->next;
	e.note = (char*) xmlNodeListGetString(doc, node->xmlChildrenNode, 1);

	xmlFreeDoc(doc);
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
 * INITIALIZERS
 */
void io_init() {
	char* home_path = getenv("HOME");

	/* check/create db directory in home */	
	asprintf(&cmd_path, "%s%s%s", home_path, "/", CMD_DIRECTORY_NAME);
	asprintf(&cmddb_path, "%s%s%s", cmd_path, "/", CMD_DB_NAME);
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

