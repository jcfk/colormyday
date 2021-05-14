#include "colormyday.h"

bool
regex_match(
	char* regex,
	char* str
) {
	regex_t reg_obj;
	regcomp(&reg_obj, regex, REG_EXTENDED);
	int reg_not_match = regexec(&reg_obj, str, 0, NULL, 0);
	regfree(&reg_obj);
	return reg_not_match ? 0 : 1;
}

int 
string_to_time(
	char* timestamp,
	struct error** err
) {
	/* Validate timestamp string */
	bool stamp_reg_match = regex_match("[0-9]*-[0-9]*-[0-9]*-[0-9]*-[0-9]*", timestamp);
	if (!stamp_reg_match) {
		init_error(err, BAD_TIMESTAMP, NONFATAL, "%s", timestamp);
		return 0;
	}

	/* Make into unix time */
	char* temp_s = strdup(timestamp);
	int data[5];
	char* token = strtok(temp_s, "-");
	int i = 0;
	while (token != NULL) {
		data[i] = atoi(token);
		token = strtok(NULL, "-");
		i = i + 1;
	}
	struct tm time;
	time.tm_year = data[0] - 1900;
	time.tm_mon = data[1] - 1;
	time.tm_mday = data[2];
	time.tm_hour = data[3];
	time.tm_min = data[4];
	time.tm_sec = 0;
	time.tm_isdst = -1;
	free(temp_s);
	return (int)mktime(&time);
}

/**************************************************************************
* INITIALIZER *************************************************************
***************************************************************************/
struct event*
malloc_event(
	void
) {
	struct event* event = malloc(sizeof(struct event));
	return event;
}


