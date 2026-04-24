#ifndef CFIRE_LIBRARY_H
#define CFIRE_LIBRARY_H

#define CFIRE_DECL_ static inline
#define CFIRE_DEF_ inline

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef enum {
    CFIRE_ARG_KIND_INTEGER = 0,
    CFIRE_ARG_KIND_DECIMAL,
    CFIRE_ARG_KIND_STRING,
} Cfire_ArgKind;

typedef enum {
    CFIRE_SUCCESS = 0,
    CFIRE_UNKNOWN_ERR,
    CFIRE_WRONG_ARGC,
    CFIRE_BAD_ALLOC,
    CFIRE_INVALID_PTR,
    CFIRE_INVALID_ARG,
    CFIRE_EMPTY_STRING
} Cfire_Error;

typedef enum {
    CFIRE_TRIM_DASHES = 1,
    CFIRE_REPL_DASHES = 1 << 1
} Cfire_Flag;

typedef union {
    long int    integer;
    double      decimal;
    const char *string;
} Cfire_ArgValue;

typedef struct {
    char           name[64];
    Cfire_ArgKind  kind;
    Cfire_ArgValue value;
} Cfire_ArgEntry;

CFIRE_DECL_ int         cfire_try_parse_integer(const char *s, long int *out);
CFIRE_DECL_ int         cfire_try_parse_decimal(const char *s, double *out);
CFIRE_DECL_ const char *cfire_remove_prefix(const char *s, char c);
CFIRE_DECL_ Cfire_Error cfire_convert_name(const char *arg_name, char *out, int flags);
CFIRE_DECL_ Cfire_Error cfire_parse_entry(const char *arg_name, const char *arg_value, Cfire_ArgEntry *out_entry, int flag);
CFIRE_DECL_ Cfire_Error cfire_parse(int argc, char **argv, Cfire_ArgEntry **out_entries, size_t *out_entry_count, int flags);
CFIRE_DECL_ const char *cfire_error_string(Cfire_Error error);
CFIRE_DECL_ void        cfire_free(Cfire_ArgEntry *entries);
CFIRE_DECL_ void        cfire_load_into(void *dest, const Cfire_ArgEntry *entry);

CFIRE_DEF_ int cfire_try_parse_integer(const char *s, long int *out) {
    if (!isdigit((unsigned char)s[0]) && s[0] != '+' && s[0] != '-') {
        return -1;
    }

    errno = 0;
    char *         end;
    const long int result = strtol(s, &end, 10);

    if (*end != 0 || errno != 0) {
        return -1;
    }

    if (out != NULL) {
        *out = result;
    }

    return 0;
}

CFIRE_DEF_ int cfire_try_parse_decimal(const char *s, double *out) {
    if (!isdigit((unsigned char)s[0]) && s[0] != '+' && s[0] != '-') {
        return -1;
    }

    errno = 0;
    char *       end;
    const double result = strtod(s, &end);

    if (*end != 0 || errno != 0) {
        return -1;
    }

    if (out != NULL) {
        *out = result;
    }

    return 0;
}

CFIRE_DEF_ const char *cfire_remove_prefix(const char *s, char c) {
    for (;; ++s) {
        if (*s != c) {
            break;
        }
    }
    return s;
}

CFIRE_DEF_ Cfire_Error cfire_convert_name(const char *arg_name, char *out, int flags) {

    Cfire_Error error_code = CFIRE_SUCCESS;
    char *      c;

    if (flags & CFIRE_TRIM_DASHES) {
        arg_name = cfire_remove_prefix(arg_name, '-');
    }

    if (arg_name[0] == '\0') {
        error_code = CFIRE_EMPTY_STRING;
        goto finish;
    }

    strcpy(out, arg_name);

    if (flags & CFIRE_REPL_DASHES) {
        for (c = out; *c != '\0'; ++c) {
            if (*c == '-') {
                *c = '_';
            }
        }
    }


finish:
    return error_code;
}

CFIRE_DEF_ Cfire_Error cfire_parse_entry(
    const char *    arg_name,
    const char *    arg_value,
    Cfire_ArgEntry *out_entry,
    int             flags
) {
    Cfire_Error error_code = CFIRE_SUCCESS;
    if (out_entry == NULL) {
        error_code = CFIRE_INVALID_PTR;
        goto finish;
    }

    if (arg_name == NULL || arg_value == NULL) {
        error_code = CFIRE_INVALID_ARG;
        goto finish;
    }

    error_code = cfire_convert_name(arg_name, out_entry->name, flags);
    if (error_code != CFIRE_SUCCESS) {
        goto finish;
    }

    if (cfire_try_parse_integer(arg_value, &out_entry->value.integer) == 0) {
        out_entry->kind = CFIRE_ARG_KIND_INTEGER;
        goto finish;
    }

    if (cfire_try_parse_decimal(arg_value, &out_entry->value.decimal) == 0) {
        out_entry->kind = CFIRE_ARG_KIND_DECIMAL;
        goto finish;
    }

    out_entry->kind         = CFIRE_ARG_KIND_STRING;
    out_entry->value.string = arg_value;

finish:
    return error_code;
}

CFIRE_DEF_ Cfire_Error cfire_parse(
    int              argc,
    char **          argv,
    Cfire_ArgEntry **out_entries,
    size_t *         out_entry_count,
    int              flags
) {

    Cfire_Error     err_code = CFIRE_SUCCESS;
    Cfire_ArgEntry *entries  = NULL;
    Cfire_ArgEntry *curr_entry;
    size_t          i;

    // argc must be and odd number
    if (!(argc & 1)) {
        err_code = CFIRE_WRONG_ARGC;
        goto error;
    }

    size_t entry_count = (argc - 1) >> 1;

    // Allocate memory
    entries = malloc(sizeof(Cfire_ArgEntry) * entry_count);
    if (entries == NULL) {
        err_code = CFIRE_BAD_ALLOC;
        goto error;
    }

    curr_entry = entries;
    for (i = 1; i < argc; i += 2, ++curr_entry) {
        err_code = cfire_parse_entry(argv[i], argv[i + 1], curr_entry, flags);
        if (err_code != CFIRE_SUCCESS) {
            goto error;
        }
    }

    // Write outputs
    if (out_entries != NULL) {
        *out_entries = entries;
    }

    if (out_entry_count != NULL) {
        *out_entry_count = entry_count;
    }

    return err_code;

error:
    if (entries != NULL) {
        free(entries);
    }

    return err_code;

}

CFIRE_DEF_ const char *cfire_error_string(Cfire_Error error) {
    switch (error) {
        case CFIRE_SUCCESS: return "Success";
        case CFIRE_WRONG_ARGC: return "Wrong argument count";
        case CFIRE_BAD_ALLOC: return "Bad memory allocation";
        case CFIRE_INVALID_PTR: return "Invalid pointer";
        case CFIRE_INVALID_ARG: return "Invalid argument";
        case CFIRE_EMPTY_STRING: return "Emtpy string";
        default: return "Unknown error";
    }
}

CFIRE_DEF_ void cfire_free(Cfire_ArgEntry *entries) {
    if (entries != NULL) {
        free(entries);
    }
}

CFIRE_DEF_ void cfire_load_into(void *dest, const Cfire_ArgEntry *entry) {
    switch (entry->kind) {
        case CFIRE_ARG_KIND_INTEGER:
            *(long int *) dest = entry->value.integer;
            break;
        case CFIRE_ARG_KIND_DECIMAL:
            *(double *) dest = entry->value.decimal;
            break;
        case CFIRE_ARG_KIND_STRING:
            *(const char **) dest = entry->value.string;
            break;
        default:
            break;
    }
}

#define CFIRE_AUTO_LOAD_BEGIN(Entries, NEntries)            \
    for (size_t i = 0; i < (NEntries); ++i) {               \
        const Cfire_ArgEntry* Cfire__local_entry = Entries + i;

#define CFIRE_AUTO_LOAD_BY_NAME(Struct, Name)               \
    if (strcmp(#Name, Cfire__local_entry->name) == 0)       \
        cfire_load_into(&Struct.Name, Cfire__local_entry);

#define CFIRE_AUTO_LOAD(Dest, Name)               \
    if (strcmp(Name, Cfire__local_entry->name) == 0)       \
    cfire_load_into(&Dest, Cfire__local_entry);

#define CFIRE_AUTO_LOAD_END }
#endif // CFIRE_LIBRARY_H
