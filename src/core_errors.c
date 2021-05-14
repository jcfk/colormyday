#include "colormyday.h"

char* error_status_strings[] = {GENERATE_ERRPOR_STATUSES(STRINGIZE)};

/***************************************************************************
* DEINITIALIZE *************************************************************
****************************************************************************/
void
free_error(
	struct error* err
) {
	free(err->message);
	free(err);
}

void
clear_error(
	struct error** err
) {
	free_error(*err);
	(*err) = NULL;
}

/****************************************************************
*   *************************************************************
*****************************************************************/
char*
error_status_to_string(
	enum error_status status
) {
	return error_status_strings[status];
}

static
char*
make_err_msg(
	char* fmt,
	va_list vargs
) {
	if (fmt == NULL) {
		return NULL;
	}
	char* msg = NULL;
	vasprintf(&msg, fmt, vargs);
	return msg;
}

void
add_error_msg(
	struct error** ptr,
	char* msg_fmt,
	...
) {
	va_list vargs;
	va_start(vargs, msg_fmt);

	char* msg = make_err_msg(msg_fmt, vargs);
	va_end(vargs);

	(*ptr)->message = msg;

}

/*********************************************************************
 * INITIALIZER *******************************************************
 *********************************************************************/
static
struct error*
malloc_error(
	void
) {
	return malloc(sizeof(struct error));

}

void
init_error(
	struct error** ptr,
	enum error_status status,
	enum fatal_status fatal,
	char* msg_fmt,
	...
) {
	char* msg = NULL;

	va_list vargs;
	va_start(vargs, msg_fmt);
	msg = make_err_msg(msg_fmt, vargs);
	va_end(vargs);

	struct error* err = malloc_error();
	err->status = status;
	err->fatal = fatal;
	err->message = msg;

	/* printf("%s", msg); */

	*ptr = err;

}

