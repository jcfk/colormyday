#include "colormyday.h"

static
void
print_err_script(
	struct error* err
) {
	char* status_string = error_status_to_string(err->status);

	printf("Error %s: %s\n", status_string, err->message);

}

void
catch_err_script(
	struct error* err
) {
	if (err) {
		print_err_script(err);
		free_error(err);
		exit_from_script(EXIT_FAILURE);

	}
}


