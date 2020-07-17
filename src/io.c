#include "colormyday.h"

char* cmd_path = NULL;
char* cmddb_path = NULL;
char* cmdgroups_path = NULL;

void make_member_group_hex_dicts() {
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlParseFile(cmdgroups_path);
	node = xmlDocGetRootElement(doc);

	member_group_dict = NULL;
	group_hex_dict = NULL;

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

		push_charpcharp_llist(name, hex, &group_hex_dict);

		while (sub_group) {
			member = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);

			push_charpcharp_llist(member, name, &member_group_dict);
			
			sub_group = sub_group->next;
		}

		group = group->next;
		i += 1;
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
		if (earlier_bound < atoi(start_time) && atoi(start_time) < later_bound) {
			if (!found) {
				asprintf(&current_event_path, "%s/%s", cmddb_path, file);
			}

			asprintf(&file, "%s%s%s", cmddb_path, "/", file);
			push_charp_llist(file, &ret);
			found = true;
		} else {
			if (found) {
				if (earlier_bound < atoi(strsep(&file_temp, "-"))) {
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

void event_to_file(char* name, struct event event) {
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
	xmlNewChild(root, NULL, BAD_CAST "note", BAD_CAST "-1");

	char* new_file;
	asprintf(&new_file, "%s/%d", cmddb_path, event.start_time);

	xmlSaveFileEnc(new_file, doc, "UTF-8");

	xmlFreeDoc(doc);

}

struct event file_to_event(char* file) {
	struct event e;
	
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlParseFile(file);
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

void begin_event(char* name) {
	struct event temp;
	temp.name = name;
	temp.start_time = current_time;
	temp.end_time = -1;

	char* new_file;
	asprintf(&new_file, "%s/%d", cmddb_path, current_time);

	event_to_file(new_file, temp);

	current_event = temp;
	current_event_path = new_file;
	
}

struct event end_current_event() {
	struct event temp = current_event;
	temp.end_time = current_time;
	current_event.name = NULL;

	event_to_file(current_event_path, temp);
	
	char* new_file;
	asprintf(&new_file, "%s-%d", current_event_path, current_time);

	rename(current_event_path, new_file);

	return temp;
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

