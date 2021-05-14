#include "colormyday.h"

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
void 
free_string_llist(
	struct string_llist* list, 
	bool malloced // strings in this llist are either all malloc'ed or not
) {
	struct string_llist* temp = list;
	struct string_llist* tempp;
	while (temp) {
		if (malloced) {
			free(temp->content);
		}
		tempp = temp;
		temp = temp->next;
		free(tempp);
	}
}

void 
free_stringstring_llist(
	struct stringstring_llist* list,
	bool malloced_1, // strings in this llist are either all malloc'ed or not
	bool malloced_2
) {
	struct stringstring_llist* temp = list;
	struct stringstring_llist* tempp;
	while (temp) {
		if (malloced_1) {
			free(temp->content_1);
		}
		if (malloced_2) {
			free(temp->content_2);
		}
		tempp = temp;
		temp = temp->next;
		free(tempp);
	}
}

/***************************************************************************
* DICTIONARIES *************************************************************
****************************************************************************/
char* 
stringstring_dict(
	struct stringstring_llist* list, 
	char* c
) {
	if (c == NULL) {
		return NULL;
	}
	char* ret = NULL;
	struct stringstring_llist* temp = list;	
	while (temp) {
		if (strcmp(temp->content_1, c) == 0) {
			ret = temp->content_2;
			break;
		}
		temp = temp->next;
	}
	return ret;
}

/*******************************************************************
* PUSH *************************************************************
********************************************************************/
void 
push_string_llist(
	char* name, 
	struct string_llist** list
) {
	struct string_llist* ret;
	ret = malloc(sizeof(struct string_llist));
	ret->content = name;
	ret->next = *list;
	*list = ret;
}


void 
push_stringstring_llist(
	char* c_1, 
	char* c_2, 
	struct stringstring_llist** list
) {
	struct stringstring_llist* ret;
	ret = malloc(sizeof(struct stringstring_llist));
	ret->content_1 = c_1;
	ret->content_2 = c_2;
	ret->next = *list;
	*list = ret;
}

