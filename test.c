#include <stdio.h>

#include "cfire.h"

struct MyArgs {
    int         a;
    double      b;
    const char *c;
    int         some_flag;
};

int main(int argc, char **argv) {
    Cfire_Entry *entries   = NULL;
    size_t       n_entries = 0;

    Cfire_Error error = cfire_parse(argc, argv, &entries, &n_entries, CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES | CFIRE_FLAGS);

    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %d\n", error);
        return 1;
    }

    // ./test --a 123456 --value-for-b 1123456.0 --c "Hello" --Fsome-flag
    struct MyArgs my_args = {.a = 0, .b = 1.0, .c = NULL, .some_flag = 0};

    Cfire_ForEachEntry(entries, n_entries, error) {
        Cfire_LoadByName(my_args, a, i);
        Cfire_LoadByName(my_args, b, d);
        Cfire_LoadByName(my_args, c, s);
        Cfire_LoadByName(my_args, some_flag, i);
    }

    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %d\n", error);
        return 1;
    }

    fprintf(stdout, "%d ", my_args.a);
    fprintf(stdout, "%f ", my_args.b);
    fprintf(stdout, "%s ", my_args.c);
    fprintf(stdout, "%d \n", my_args.some_flag);

    cfire_free(entries);

    return 0;
}
