# CFire - Simple Argument Parse Library in C

> [!WARNING]
> The work is still in progress. Anything may change or be removed.
> This file may contain outdated information because I have not
> updated it for latest changes.

This is a simple header-only C-library for extracting command line argument names and values.
The command line arguments are parsed into an array of `Cfire_Entry` structs, which has a
name, and value-kind tag, and a union for the value.

In brief, you call `cfire_parse(argc, argv, &entries, &n_entries, flags)` and to
extract the command line arguments, where `flags` controls the behavior of
the parser. See "Parse Flags (Not Flag Entries)" section.

## Build

This is an stb-like single-file library. Define `CFIRE_IMPLEMENTATION` before
including `cfire.h` to contain the function definitions. Otherwise, only the
declarations will be included, and you need to link libcfire.a.

## The `Cfire_Entry` Struct

This is a name-value pair, with the name stored in a null-terminated `char[64]` array,
and value expressed in a tagged union (`kind` and `value`). The field `kind` is just an
enumeration type to tell the value type (kind) that is actually being held by `value`, and
you can access the `value` correspondingly.

## Flag and Non-Flag Entries

The tagged union now supports four values:
     CFIRE_ARG_KIND_INTEGER (unsigned long)
     CFIRE_ARG_KIND_FLOATING (double)
     CFIRE_ARG_KIND_STRING (const char )
     CFIRE_ARG_KIND_FLAG (int)

For a non-flag entry, the union value is parsed in the order of INTEGER, FLOATING,
AND STRING. Should the program fails to parse the value, it falls back to the following,
and for STRING type, the extraction always succeeds, even if the `argv[?]` is
a null string.

Non-flag entries are given in name-value pairs, such as `--first-argument 1`. This
results in an entry `"first_argument", INTEGER, 1`.

Flag entries are given with a special prefix `"--F"`. For example, `--Fsome-flag`
results in `"some_flag", FLAG, 1`. The prefix for flag arguments and the flag value are
configurable by defining the macro `CFIRE_FLAG_ENTRY_PREFIX` and `CFIRE_FLAG_VALUE`,
respectively, before including this header file.

## Parser Flags (Not Flag Entries)

The parser's behavior is configurable by `flags` argument. In the above examples,
we set `flags = CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES | CFIRE_FLAGS | CFIRE_ZERO`.

The flag `CFIRE_REPL_DASHES` tells the parser to replace dashes (`-`) with underscores
(`_`). Therefore, you see that `--first-argument` turns into `"first_argument"`. The
starting dashes are removed due to `CFIRE_TRIM_DASHES`. Should this flag be not specified,
the resulting name would be `"__first_argument"`.

The flag `CFIRE_FLAGS` tells the parser to treat arguments starting with `"--F"` (or your
custom `CFIRE_FLAG_ENTRY_PREFIX`) value as a flag entry. Flag entries contains only one
command line argument such as `--Fsome-flag`. If `CFIRE_FLAGS` is not OR'd to the flags
parameter, then the parser will expect some value like `--Fsome-flag <some-value>`, or an
error will occur. Command-line arguments for flags cannot be given between the name-value
pair of a non-flag entry.

The last available flag is `CFIRE_ZERO`, which tells the parse to start parsing `argv` from
`argv[0]`. Otherwise, `argv[0]` will be ignored. In either cases, `argc` should always be the
same value (the length of `argv`).

## Memory

You don't need to allocate for the `Cfire_Entry` array; they will be allocated by the parser.
However, you need to free the allocated memory by calling `cfire_free(entries)`.

## Error Handling

Should the parser fail to parse the command line argument, an error code will be returned.
Otherwise, the parser will return `CFIRE_SUCCESS`. Usage `cfire_error_string` to convert
error codes into error strings.

## Macros

You may use `CFire_ForEachEntry` and `CFire_LoadByName` to extract `CFire_Entry`'s value
into a variable:

> [!WARNING]
> I am not sure if this is the most straightforward and safe way to do this.
> But I think it serves my current purpose for now (in a private project).

```c

#include <stdio.h>
#include "cfire.h"

// ./test --a 123456 --b 1123456.0 --c "Hello" --Fsome-flag
int main(int argc, char **argv) {
    Cfire_Entry *entries   = NULL;
    size_t       n_entries = 0;

    Cfire_Error error = cfire_parse(argc, argv, &entries, &n_entries, CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES | CFIRE_FLAGS);

    if (error != CFIRE_SUCCESS) {
        fprintf(stderr, "Error: %d\n", error);
        return 1;
    }

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

```
