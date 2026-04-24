/**
 * This is a simple header-only C-library for extracting command line argument names and values.
 * The command line arguments are parsed into an array of `Cfire_Entry` structs, which has a
 * name, and value-kind tag, and a union for the value.
 *
 * In brief, you call `cfire_parse(argc, argv, &entries, &n_entries, flags)` and to
 * extract the command line arguments, where `flags` controls the behavior of
 * the parser. See "Parse Flags (Not Flag Entries)" section.
 *
 *
 * The Cfire_Entry Struct
 * ----------------------
 *
 * This is a name-value pair, with the name stored in a null-terminated `char[64]` array,
 * and value expressed in a tagged union (`kind` and `value`). The field `kind` is just an
 * enumeration type to tell the value type (kind) that is actually being held by `value`, and
 * you can access the `value` correspondingly.
 *
 *
 * Flag and Non-Flag Entries
 * -------------------------
 *
 * The tagged union now supports four values:
 *      * CFIRE_ARG_KIND_INTEGER (unsigned long)
 *      * CFIRE_ARG_KIND_FLOATING (double)
 *      * CFIRE_ARG_KIND_STRING (const char *)
 *      * CFIRE_ARG_KIND_FLAG (int)
 *
 * For a non-flag entry, the union value is parsed in the order of INTEGER, FLOATING,
 * AND STRING. Should the program fails to parse the value, it falls back to the following,
 * and for STRING type, the extraction always succeeds, even if the `argv[?]` is
 * a null string.
 *
 * Non-flag entries are given in name-value pairs, such as `--first-argument 1`. This
 * results in an entry `"first_argument", INTEGER, 1`.
 *
 * Flag entries are given with a special prefix `"--F"`. For example, `--Fsome-flag`
 * results in `"some_flag", FLAG, 1`. The prefix for flag arguments and the flag value are
 * configurable by defining the macro `CFIRE_FLAG_ENTRY_PREFIX` and `CFIRE_FLAG_VALUE`,
 * respectively, before including this header file.
 *
 *
 * Parser Flags (Not Flag Entries)
 * -------------------------------
 *
 * The parser's behavior is configurable by `flags` argument. In the above examples,
 * we set `flags = CFIRE_REPL_DASHES | CFIRE_TRIM_DASHES | CFIRE_FLAGS | CFIRE_ZERO`.
 *
 * The flag `CFIRE_REPL_DASHES` tells the parser to replace dashes (`-`) with underscores
 * (`_`). Therefore, you see that `--first-argument` turns into `"first_argument"`. The
 * starting dashes are removed due to `CFIRE_TRIM_DASHES`. Should this flag be not specified,
 * the resulting name would be `"__first_argument"`.
 *
 * The flag `CFIRE_FLAGS` tells the parser to treat arguments starting with `"--F"` (or your
 * custom `CFIRE_FLAG_ENTRY_PREFIX`) value as a flag entry. Flag entries contains only one
 * command line argument such as `--Fsome-flag`. If `CFIRE_FLAGS` is not OR'd to the flags
 * parameter, then the parser will expect some value like `--Fsome-flag <some-value>`, or an
 * error will occur. Command-line arguments for flags cannot be given between the name-value
 * pair of a non-flag entry.
 *
 * The last available flag is `CFIRE_ZERO`, which tells the parse to start parsing `argv` from
 * `argv[0]`. Otherwise, `argv[0]` will be ignored. In either cases, `argc` should always be the
 * same value (the length of `argv`).
 *
 *
 * Memory
 * ------
 * You don't need to allocate for the `Cfire_Entry` array; they will be allocated by the parser.
 * However, you need to free the allocated memory by calling `cfire_free(entries)`.
 *
 *
 * Error Handling
 * --------------
 * Should the parser fail to parse the command line argument, an error code will be returned.
 * Otherwise, the parser will return `CFIRE_SUCCESS`. Usage `cfire_error_string` to convert
 * error codes into error strings.
 *
 */

#ifndef CFIRE_LIBRARY_H
#define CFIRE_LIBRARY_H

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef CFIRE_FLAG_ENTRY_PREFIX
#define CFIRE_FLAG_ENTRY_PREFIX "--F"
#endif

#ifndef CFIRE_FLAG_VALUE
#define CFIRE_FLAG_VALUE 1
#endif

#define CFIRE_DECL_ static inline
#define CFIRE_DEF_ inline

#ifdef __cplusplus
#define CFIRE_DECLARATION_ONLY
#endif

#define CFIRE_ARG_KIND_INTEGER_VALUE(Entry) (Entry)->value.integer
#define CFIRE_ARG_KIND_FLOATING_VALUE(Entry) (Entry)->value.floating
#define CFIRE_ARG_KIND_STRING_VALUE(Entry) (Entry)->value.string
#define CFIRE_ARG_KIND_FLAG_VALUE(Entry) (Entry)->value.flag
#define CFIRE_GET_UNION_VALUE_(Kind, Entry) CFIRE_ARG_KIND_##Kind##_VALUE(Entry)

typedef enum {
    CFIRE_ARG_KIND_INTEGER = 0,
    CFIRE_ARG_KIND_FLOATING,
    CFIRE_ARG_KIND_STRING,
    CFIRE_ARG_KIND_FLAG
} Cfire_ArgKind;

typedef enum {
    CFIRE_SUCCESS = 0,
    CFIRE_UNKNOWN_ERR,
    CFIRE_WRONG_ARGC,
    CFIRE_BAD_ALLOC,
    CFIRE_INVALID_PTR,
    CFIRE_INVALID_ARG,
    CFIRE_EMPTY_STRING,
    CFIRE_WRONG_TYPE
} Cfire_Error;

typedef enum {
    CFIRE_TRIM_DASHES = 1,
    CFIRE_REPL_DASHES = 1 << 1,
    CFIRE_FLAGS       = 1 << 2,
    CFIRE_ZERO        = 1 << 3
} Cfire_Flag;

typedef union {
    long int    integer;
    double      floating;
    const char *string;
    int         flag;
} Cfire_ArgValue;

typedef struct {
    char           name[64];
    Cfire_ArgKind  kind;
    Cfire_ArgValue value;
} Cfire_Entry;

typedef int (*Cfire_Predicate_t)(const char *, void *, void *);

#ifdef __cplusplus
extern "C" {
#endif

CFIRE_DECL_ int         cfire_try_parse_integer(const char *s, long int *out);
CFIRE_DECL_ int         cfire_try_parse_floating(const char *s, double *out);
CFIRE_DECL_ const char *cfire_remove_prefix(const char *s, char c);
CFIRE_DECL_ Cfire_Error cfire_convert_name(const char *arg_name, char *out, int flags);
CFIRE_DECL_ Cfire_Error cfire_parse_entry(const char *arg_name, const char *arg_value, Cfire_Entry *out_entry, int flag);
CFIRE_DECL_ Cfire_Error cfire_parse(int argc, char **argv, Cfire_Entry **out_entries, size_t *out_entry_count, int flags);
CFIRE_DECL_ const char *cfire_error_string(Cfire_Error error);
CFIRE_DECL_ void        cfire_free(Cfire_Entry *entries);
CFIRE_DECL_ size_t      cfire_count_if(int n, char **cv, Cfire_Predicate_t, void *arg);
CFIRE_DECL_ int         cfire_starts_with(const char *s, void *pprfx, void *offset);

CFIRE_DECL_ Cfire_Error cfire_loadi(int *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadl(long *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadll(long long *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadu(unsigned int *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadul(unsigned long *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadull(unsigned long long *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadd(double *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loadf(float *dest, const Cfire_Entry *entry);
CFIRE_DECL_ Cfire_Error cfire_loads(const char **dest, const Cfire_Entry *entry);

#ifdef __cplusplus
}
#endif
#ifndef CFIRE_DECLARATION_ONLY

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

CFIRE_DEF_ int cfire_try_parse_floating(const char *s, double *out) {
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
    const char * arg_name,
    const char * arg_value,
    Cfire_Entry *out_entry,
    int          flags
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

    if (cfire_try_parse_floating(arg_value, &out_entry->value.floating) == 0) {
        out_entry->kind = CFIRE_ARG_KIND_FLOATING;
        goto finish;
    }

    out_entry->kind         = CFIRE_ARG_KIND_STRING;
    out_entry->value.string = arg_value;

finish:
    return error_code;
}

CFIRE_DEF_ Cfire_Error cfire_parse(
    int           argc,
    char **       argv,
    Cfire_Entry **out_entries,
    size_t *      out_entry_count,
    int           flags
) {

    Cfire_Error  err_code = CFIRE_SUCCESS;
    Cfire_Entry *entries  = NULL;
    Cfire_Entry *curr_entry;
    size_t       i;
    size_t       entry_cnt;
    size_t       fentry_cnt;
    size_t       act_argc = argc - ((flags & CFIRE_ZERO) ? 0 : 1);

    if (!(flags & CFIRE_FLAGS)) {

        if (act_argc & 1) {
            err_code = CFIRE_WRONG_ARGC;
            goto error;
        }

        entry_cnt = act_argc >> 1;

        entries = malloc(sizeof(Cfire_Entry) * entry_cnt);
        if (entries == NULL) {
            err_code = CFIRE_BAD_ALLOC;
            goto error;
        }

        curr_entry = entries;
        for (i = (flags & CFIRE_ZERO) ? 0 : 1; i < argc; i += 2, ++curr_entry) {
            err_code = cfire_parse_entry(argv[i], argv[i + 1], curr_entry, flags);
            if (err_code != CFIRE_SUCCESS) {
                goto error;
            }
        }

    } else {

        fentry_cnt = cfire_count_if(
            (int) act_argc,
            argv + ((flags & CFIRE_ZERO) ? 0 : 1),
            cfire_starts_with,
            CFIRE_FLAG_ENTRY_PREFIX
        );

        if ((act_argc - fentry_cnt) & 1) {
            err_code = CFIRE_WRONG_ARGC;
            goto error;
        }

        entry_cnt = ((act_argc - fentry_cnt) >> 1) + fentry_cnt;

        entries = malloc(sizeof(Cfire_Entry) * entry_cnt);
        if (entries == NULL) {
            err_code = CFIRE_BAD_ALLOC;
            goto error;
        }

        curr_entry = entries;
        size_t offset;
        int    p = 0; // indicate whether we are in the middle of a name-value pair
        for (i = (flags & CFIRE_ZERO) ? 0 : 1; i < argc; ++i) {

            // Parse flag entries
            if (cfire_starts_with(argv[i], CFIRE_FLAG_ENTRY_PREFIX, &offset)) {

                if (p != 0) {
                    err_code = CFIRE_INVALID_ARG;
                    goto error;
                }

                err_code = cfire_convert_name(argv[i] + offset, curr_entry->name, flags);
                if (err_code != CFIRE_SUCCESS) {
                    goto error;
                }

                curr_entry->kind       = CFIRE_ARG_KIND_FLAG;
                curr_entry->value.flag = CFIRE_FLAG_VALUE;

                ++curr_entry;
                continue;

            }

            if (p == 0) {
                p = 1;
                continue;
            }

            p = 0;

            err_code = cfire_parse_entry(argv[i - 1], argv[i], curr_entry, flags);
            if (err_code != CFIRE_SUCCESS) {
                goto error;
            }
            ++curr_entry;

        }
    }


    // Write outputs
    if (out_entries != NULL) {
        *out_entries = entries;
    }

    if (out_entry_count != NULL) {
        *out_entry_count = entry_cnt;
    }

    return err_code;

error:
    if (entries != NULL) {
        free(entries);
    }

    return err_code;

}


CFIRE_DEF_ size_t cfire_count_if(int n, char **cv, Cfire_Predicate_t pred, void *arg) {
    size_t retval = 0;
    for (int i = 0; i < n; ++i) {
        retval += pred(cv[i], arg, NULL) == 1;
    }

    return retval;
}

CFIRE_DEF_ int cfire_starts_with(const char *s, void *pprfx, void *offset) {
    const char *prefix = pprfx;

    size_t idx = 0;
    for (; prefix[idx] != '\0'; ++idx) {
        if (s[idx] == '\0') {
            return 0;
        }

        if (prefix[idx] != s[idx]) {
            return 0;
        }
    }

    if (offset != NULL) {
        *(size_t *) offset = idx;
    }

    return 1;
}

CFIRE_DEF_ const char *cfire_error_string(Cfire_Error error) {
    switch (error) {
        case CFIRE_SUCCESS: return "Success";
        case CFIRE_WRONG_ARGC: return "Wrong argument count";
        case CFIRE_BAD_ALLOC: return "Bad memory allocation";
        case CFIRE_INVALID_PTR: return "Invalid pointer";
        case CFIRE_INVALID_ARG: return "Invalid argument";
        case CFIRE_EMPTY_STRING: return "Emtpy string";
        case CFIRE_WRONG_TYPE: return "Wrong value type";
        default: return "Unknown error";
    }
}

CFIRE_DEF_ void cfire_free(Cfire_Entry *entries) {
    if (entries != NULL) {
        free(entries);
    }
}

/* vvv For loading values vvv */

#define CFIRE_ARG_ASSIGN_(Kind, Type) \
    case CFIRE_ARG_KIND_##Kind: \
        *(Cfire_Macro_Internal_Dest) = (Type) CFIRE_GET_UNION_VALUE_(Kind, Cfire_Macro_Internal_Entry); \
        return CFIRE_SUCCESS

#define CFIRE_DEFINE_TYPED_CONVERSION_BEGIN_(Type, Code) \
    CFIRE_DEF_ Cfire_Error cfire_load##Code(Type* dest, const Cfire_Entry* entry) { \
        Type* Cfire_Macro_Internal_Dest = dest; \
        const Cfire_Entry*  Cfire_Macro_Internal_Entry = entry;\
        switch(Cfire_Macro_Internal_Entry->kind) {

#define CFIRE_DEFINE_TYPED_CONVERSION_END_ default: return CFIRE_WRONG_TYPE; } }

#define CFIRE_DEFINE_NUMERICAL_CONVERSION_(Type, Code) \
    CFIRE_DEFINE_TYPED_CONVERSION_BEGIN_(Type, Code) \
        CFIRE_ARG_ASSIGN_(INTEGER, Type);\
        CFIRE_ARG_ASSIGN_(FLOATING, Type);\
        CFIRE_ARG_ASSIGN_(FLAG, Type);\
    CFIRE_DEFINE_TYPED_CONVERSION_END_

#define CFIRE_DEFINE_STRING_CONVERSION_(Type, Code) \
    CFIRE_DEFINE_TYPED_CONVERSION_BEGIN_(Type, Code) \
        CFIRE_ARG_ASSIGN_(STRING, Type);\
    CFIRE_DEFINE_TYPED_CONVERSION_END_

CFIRE_DEFINE_NUMERICAL_CONVERSION_(int, i);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(long, l);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(long long, ll);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(unsigned int, u);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(unsigned long, ul);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(unsigned long long, ull);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(double, d);
CFIRE_DEFINE_NUMERICAL_CONVERSION_(float, f);
CFIRE_DEFINE_STRING_CONVERSION_(const char*, s);

#define Cfire_ForEachEntry(Entries, NEntries, ErrorCode) \
    size_t Cfire_Macro_Internal_Index = 0;  \
    Cfire_Error* Cfire_Macro_Internal_ErrorCodePtr = &(ErrorCode); \
    const Cfire_Entry* Cfire_Macro_Internal_Entries = (Entries);      \
    for (; Cfire_Macro_Internal_Index < (NEntries); ++Cfire_Macro_Internal_Index)

#define Cfire_Load(OutVariable, Name, TypeCode) \
    if (strcmp(Cfire_Macro_Internal_Entries[Cfire_Macro_Internal_Index].name, (Name)) == 0) { \
      *Cfire_Macro_Internal_ErrorCodePtr = cfire_load##TypeCode(&(OutVariable), &Cfire_Macro_Internal_Entries[Cfire_Macro_Internal_Index]); \
      if (*Cfire_Macro_Internal_ErrorCodePtr != CFIRE_SUCCESS) break; \
    }

#define Cfire_LoadByName(OutStruct, Name, TypeCode) Cfire_Load((OutStruct).Name, #Name, TypeCode)

#endif // NOT CFIRE_DECLARATION_ONLY

#endif // CFIRE_LIBRARY_H
