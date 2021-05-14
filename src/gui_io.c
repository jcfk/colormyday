#include "colormyday.h"
#include "gui.h"

/*************************************************************************
* EVENT DATA *************************************************************
**************************************************************************/
struct string_llist* 
get_events_between(
	struct tm earlier_bound_tm, 
	struct tm later_bound_tm
) {
	int earlier_bound, later_bound;
	if (earlier_bound_tm.tm_year != 1970) { // 1970???
		earlier_bound = mktime(&earlier_bound_tm);
	} else {
		earlier_bound = -1;
	}
	if (later_bound_tm.tm_year != 1970) {
		later_bound = mktime(&later_bound_tm);
	} else {
		later_bound = -1;
	}

	struct dirent** namelist;
	struct string_llist* ret = NULL;
	char* file;
	char* file_temp;
	char* file_tempp;
	char* start_time;
	bool found = false;
	bool done = false;
	int d = scandir(cmddata_path, &namelist, NULL, alphasort); // use filter_out_hidden?
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
			if (earlier_bound < atoi(start_time) 
				&& (atoi(start_time) < later_bound || later_bound == -1)
			) {
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

char*
cursor_event_path(
	struct error** err
) {
	char* ret = NULL;
	if (cursor_event == NULL) {
		init_error(err, NO_EVENT_SELECTED, NONFATAL, NULL);
		return NULL;
	} else if (cursor_event->event->end_time == TILL_NOW) {
		asprintf(&ret, "%s/%d", cmddata_path, cursor_event->event->start_time);
	} else {
		asprintf(&ret, 
			"%s/%d-%d", 
			cmddata_path, 
			cursor_event->event->start_time, 
			cursor_event->event->end_time
		);
	}
	return ret;
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
void 
make_member_group_hex_dicts(
	/* struct stringstring_llist** member_group_dict, */ 
	/* struct stringstring_llist** group_hex_dict, */
	struct error** err
) {
	xmlDocPtr doc = xmlReadFile(cmdgroups_path, NULL, 256);
	if (doc == NULL) {
		init_error(err, BAD_XML, FATAL, "in %s", cmdgroups_path);
		return;
	}
	xmlNodePtr node = xmlDocGetRootElement(doc);
	int i = 50;
	char *name, *hex, *member;
	xmlNodePtr sub_group;
	xmlNodePtr group = node->xmlChildrenNode;
	/* return some error when we cross 256 groups or so */
	while (group && i < 256) {
		sub_group = group->xmlChildrenNode;
		name = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);
		sub_group = sub_group->next;
		hex = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);
		sub_group = sub_group->next->xmlChildrenNode;
		push_stringstring_llist(name, hex, &group_hex_dict);
		while (sub_group) {
			member = (char*) xmlNodeListGetString(doc, sub_group->xmlChildrenNode, 1);
			push_stringstring_llist(member, name, &member_group_dict);
			sub_group = sub_group->next;
		}
		i += 1;
		group = group->next;
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
}


