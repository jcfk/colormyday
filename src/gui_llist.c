#include "colormyday.h"
#include "gui.h"

/****************************************************************************
* DEINITIALIZER *************************************************************
*****************************************************************************/
void
free_stringint_llist(
	struct stringint_llist* list,
	bool malloced
) {
	struct stringint_llist* temp = list;
	struct stringint_llist* tempp;
	while (temp) {
		if (malloced) {
			free(temp->c_content);
		}
		tempp = temp;
		temp = temp->next;
		free(tempp);
	}
}

void 
free_dsp_eventp_llist(
	struct dsp_eventp_llist* list
) {
	struct dsp_eventp_llist* temp = list;
	struct dsp_eventp_llist* tempp;
	while (temp) {
		tempp = temp;
		temp = temp->next;
		free(tempp->dsp_event->event->name);
		free(tempp->dsp_event->event->note);
		free(tempp->dsp_event->event);
		free(tempp->dsp_event);
		free(tempp);
	}
}

/* void free_eventp_llist(struct eventp_llist* list) { */

/* } */

/*************************************************************************
* DICTIONARY *************************************************************
**************************************************************************/
int 
stringint_dict(
	struct stringint_llist* list, 
	char* c // c??
) {
	if (c == NULL) {
		return -1;
	}
	int ret = 0;
	struct stringint_llist* temp = list;
	while (temp) {
		if (strcmp(temp->c_content, c) == 0) {
			ret = temp->i_content;
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
push_stringint_llist(
	char* c_content, 
	int i_content, 
	struct stringint_llist** list
) {
	struct stringint_llist* ret;
	ret = malloc(sizeof(struct stringint_llist));
	ret->c_content = c_content;
	ret->i_content = i_content;
	ret->next = *list;
	*list = ret;
}

void 
push_dsp_eventp_llist(
	struct dsp_event* dsp_event, 
	struct dsp_eventp_llist** list
) {
	struct dsp_eventp_llist* ret;
	ret = malloc(sizeof(struct dsp_eventp_llist));
	ret->dsp_event = dsp_event;
	ret->next = *list;
	*list = ret;
}

void 
push_eventp_llist(
	struct event* event, 
	struct eventp_llist** list
) {
	struct eventp_llist* ret;
	ret = malloc(sizeof(struct eventp_llist));
	ret->event = event;
	ret->next = *list;
	*list = ret;
}



