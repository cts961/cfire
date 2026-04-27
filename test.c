#include <stdio.h>
#define CFIRE_IMPLEMENTATION
#include "cfire.h"

struct MyArgs {
    int         a;
    double      b;
    const char *c;
    int         some_flag;
    const char* another_string;
    const char* yet_another_string;
};

struct MyArgs my_args = {.a = 0, .b = 1.0, .c = NULL, .some_flag = 0};

int main(int argc, char **argv) {
    Cfire_Entry *entries   = NULL;
    size_t       n_entries = 0;
	int          err_index = -1;
    
    Cfire_Error error;
    error = cfire_parse(argc, argv, &entries, &n_entries, &err_index, CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES | CFIRE_FLAGS | CFIRE_COPY_STRINGS);
    
    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %s\n", cfire_error_string(error));
		fprintf(stderr, "Error Index: %d\n", err_index);
        return 1;
    }

    Cfire_ForEachEntry(entries, n_entries, error) {
        Cfire_LoadByName(my_args, a, i);
        Cfire_LoadByName(my_args, b, d);
        Cfire_LoadByName(my_args, c, s);
        Cfire_LoadByName(my_args, some_flag, i);
        Cfire_LoadByName(my_args, another_string, s);
        Cfire_LoadByName(my_args, yet_another_string, s);
    }

    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %s\n", cfire_error_string(error));
		fprintf(stderr, "Error Index: %d\n", err_index);
        return 1;
    }
    
    fprintf(stdout, "%d ", my_args.a);
    fprintf(stdout, "%f ", my_args.b);
    fprintf(stdout, "%s ", my_args.c);
    fprintf(stdout, "%s ", my_args.another_string);
    fprintf(stdout, "%s ", my_args.yet_another_string);
    fprintf(stdout, "%d \n", my_args.some_flag);

    cfire_free(entries);

    return 0;
}
