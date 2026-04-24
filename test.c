#include <stdio.h>

#include "cfire.h"

struct MyArgs {
    int         a;
    double      b;
    const char *c;
};

int main(int argc, char **argv) {
    Cfire_ArgEntry *entries   = NULL;
    size_t          n_entries = 0;

    Cfire_Error error = cfire_parse(argc, argv, &entries, &n_entries, CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES);

    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %d\n", error);
        return 1;
    }

    struct MyArgs my_args = {.a = 0, .b = 1.0, .c = NULL};

    CFIRE_AUTO_LOAD_BEGIN(entries, n_entries)
        CFIRE_AUTO_LOAD_BY_NAME(my_args, a);
        CFIRE_AUTO_LOAD(my_args.b, "value_for_b");
        CFIRE_AUTO_LOAD_BY_NAME(my_args, c);
    CFIRE_AUTO_LOAD_END

    fprintf(stdout, "%d ", my_args.a);
    fprintf(stdout, "%f ", my_args.b);
    fprintf(stdout, "%s \n", my_args.c);

    cfire_free(entries);

    return 0;
}
