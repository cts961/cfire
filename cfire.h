/**
 * Copyright © 2026 Tianshu Cai <cts961@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef CFIRE_LIBRARY_H
#define CFIRE_LIBRARY_H

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

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
    CFIRE_TRIM_DASHES  = 1,
    CFIRE_REPL_DASHES  = 1 << 1,
    CFIRE_FLAGS        = 1 << 2,
    CFIRE_ZERO         = 1 << 3,
    CFIRE_COPY_STRINGS = 1 << 4
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

typedef struct {
    void   *data;
    size_t  size;
    size_t  capacity;
} Cfire_Data;


#ifdef __cplusplus
extern "C" {
#endif

CFIRE_DECL_ int         cfire_try_parse_integer(const char *s, long int *out);
CFIRE_DECL_ int         cfire_try_parse_floating(const char *s, double *out);
CFIRE_DECL_ const char *cfire_remove_prefix(const char *s, char c);
CFIRE_DECL_ Cfire_Error cfire_convert_name(const char *arg_name, char *out, int flags);
CFIRE_DECL_ Cfire_Error cfire_parse_entry(const char *arg_name, const char *arg_value, Cfire_Entry *out_entry, Cfire_Data *data, int flag);
CFIRE_DECL_ Cfire_Error cfire_parse(int argc, char **argv, Cfire_Entry **out_entries, size_t *out_entry_count, int* err_index, int flags);
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

CFIRE_DECL_ Cfire_Error cfire_data_new(Cfire_Data **cfire_data);
CFIRE_DECL_ void        cfire_data_free(Cfire_Data *cfire_data);
CFIRE_DECL_ Cfire_Error cfire_data_append(Cfire_Data *cfire_data, const void *src, size_t n, void** result);
CFIRE_DECL_ Cfire_Error cfire_data_expand(Cfire_Data *cfire_data, size_t target);

#define CFIRE_DATA_INITIAL_CAPACITY 256

#ifdef __cplusplus
}
#endif
#ifndef CFIRE_DECLARATION_ONLY

CFIRE_DEF_ Cfire_Error cfire_data_new(Cfire_Data **cfire_data) {
    Cfire_Error err_code = CFIRE_SUCCESS;
    Cfire_Data* retval;

    if (cfire_data == NULL) {
        err_code = CFIRE_INVALID_PTR;
        goto error;
    }

    retval = malloc(sizeof(Cfire_Data));
    if (retval == NULL) {
        err_code = CFIRE_BAD_ALLOC;
        goto error;
    } 

    retval->data = malloc(CFIRE_DATA_INITIAL_CAPACITY);
    if (retval->data == NULL) {
        err_code = CFIRE_BAD_ALLOC;
        goto error;
    }

    retval->size = 0;
    retval->capacity = CFIRE_DATA_INITIAL_CAPACITY;

    *cfire_data = retval;
    return CFIRE_SUCCESS;

error:
    cfire_data_free(retval);

    return err_code;
}

CFIRE_DEF_ void cfire_data_free(Cfire_Data *cfire_data) {
   if (cfire_data == NULL) return;

   if (cfire_data->data != NULL) {
       free(cfire_data->data);
   }

   free(cfire_data);
}

CFIRE_DEF_ Cfire_Error cfire_data_append(Cfire_Data *cfire_data, const void *src, size_t n, void** result) {
    Cfire_Error err_code = CFIRE_SUCCESS;

    if (cfire_data == NULL) {
        err_code = CFIRE_INVALID_PTR;
        goto error;
    }

    if (cfire_data->size + n > cfire_data->capacity) {
        err_code = cfire_data_expand(cfire_data, cfire_data->size + n);
        if (err_code != CFIRE_SUCCESS) {
            goto error;
        }
    }

    if (result != NULL) {
        *result = cfire_data->data + cfire_data->size;
    }

    memcpy(cfire_data->data + cfire_data->size, src, n);
    cfire_data->size += n;

    return CFIRE_SUCCESS;

error:
    return err_code;
}

CFIRE_DEF_ Cfire_Error cfire_data_expand(Cfire_Data *cfire_data, size_t target) {
    Cfire_Error err_code = CFIRE_SUCCESS;
    size_t      new_capacity;

    if (cfire_data == NULL) {
        err_code = CFIRE_INVALID_PTR;
        goto error;
    }
    
    if (cfire_data->capacity >= target) {
        return CFIRE_SUCCESS;
    }

    new_capacity = cfire_data->capacity << 1;
    new_capacity += (new_capacity == 0);
    for (;new_capacity < target; new_capacity >>= 1);

    void* new_data = malloc(new_capacity);
    if (new_data == NULL) {
        new_data = malloc(target);
        if (new_data == NULL) {
            err_code = CFIRE_BAD_ALLOC;
            goto error;
        }
    }

    memcpy(new_data, cfire_data->data, cfire_data->size);
    free(cfire_data->data);

    cfire_data->data = new_data;
    cfire_data->capacity = new_capacity;
    return CFIRE_SUCCESS;

error:
    return err_code;
}

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
    for (;*s == c && *s != '\0';++s);
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
    const char  *arg_name,
    const char  *arg_value,
    Cfire_Entry *out_entry,
    Cfire_Data  *data,
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

    out_entry->kind = CFIRE_ARG_KIND_STRING;
    if (flags & CFIRE_COPY_STRINGS) {
        void *result;
        error_code = cfire_data_append(data, arg_value, strlen(arg_value), &result);
        if (error_code != CFIRE_SUCCESS) {
            goto finish;
        }
        out_entry->value.string = result;
    } else {
        out_entry->value.string = arg_value;
    }

finish:
    return error_code;
}

#define CFIRE_NEEDS_ALLOCATE_DATA_(flag) ((flag) | CFIRE_COPY_STRINGS)

CFIRE_DEF_ Cfire_Error cfire_parse(
    int           argc,
    char **       argv,
    Cfire_Entry **out_entries,
	size_t *      out_entry_count,
	int *         out_err_index,
    int           flags
) {

    Cfire_Error  err_code = CFIRE_SUCCESS;
    Cfire_Entry *entries  = NULL;
    Cfire_Entry *curr_entry;
    size_t       i;
    size_t       entry_cnt;
    size_t       fentry_cnt;
    size_t       act_argc = argc - ((flags & CFIRE_ZERO) ? 0 : 1);
	int          err_index = -1;
    void*        ptr;

    if (!(flags & CFIRE_FLAGS)) {

        if (act_argc & 1) {
            err_code = CFIRE_WRONG_ARGC;
            goto error;
        }

        entry_cnt = act_argc >> 1;

        ptr = malloc(sizeof(Cfire_Entry) * entry_cnt + sizeof(Cfire_Data *));
        if (ptr == NULL) {
            err_code = CFIRE_BAD_ALLOC;
            goto error;
        }
        entries = ptr + sizeof(Cfire_Data *);

        if (CFIRE_NEEDS_ALLOCATE_DATA_(flags)) {
            err_code = cfire_data_new((Cfire_Data **) ptr);
            if (err_code != CFIRE_SUCCESS) {
                goto error;
            }
        } else {
            *(Cfire_Data **) ptr = NULL;
        }

        curr_entry = entries;
        for (i = (flags & CFIRE_ZERO) ? 0 : 1; i < argc; i += 2, ++curr_entry) {
            err_code = cfire_parse_entry(argv[i], argv[i + 1], curr_entry, *(Cfire_Data **) ptr, flags);
            if (err_code != CFIRE_SUCCESS) {
				err_index = i;
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

        ptr = malloc(sizeof(Cfire_Entry) * entry_cnt + sizeof(Cfire_Data *));
        if (ptr == NULL) {
            err_code = CFIRE_BAD_ALLOC;
            goto error;
        }
        entries = ptr + sizeof(Cfire_Data *);

        if (CFIRE_NEEDS_ALLOCATE_DATA_(flags)) {
            err_code = cfire_data_new((Cfire_Data **) ptr);
            if (err_code != CFIRE_SUCCESS) {
                goto error;
            }
        } else {
            *(Cfire_Data **) ptr = NULL;
        }

        curr_entry = entries;
        size_t offset;
        int    p = 0; // indicate whether we are in the middle of a name-value pair
        for (i = (flags & CFIRE_ZERO) ? 0 : 1; i < argc; ++i) {

            // Parse flag entries
            if (cfire_starts_with(argv[i], CFIRE_FLAG_ENTRY_PREFIX, &offset)) {

                if (p != 0) {
                    err_code = CFIRE_INVALID_ARG;
					err_index = i;
                    goto error;
                }

                err_code = cfire_convert_name(argv[i] + offset, curr_entry->name, flags);
                if (err_code != CFIRE_SUCCESS) {
					err_index = i;
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

            err_code = cfire_parse_entry(argv[i - 1], argv[i], curr_entry, *(Cfire_Data **) ptr, flags);
            if (err_code != CFIRE_SUCCESS) {
				err_index = i - 1;
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
    cfire_free(entries);

	if (out_err_index != NULL) {
		*out_err_index = err_index;
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
        void* ptr = (void *) entries - sizeof(Cfire_Data *);
        cfire_data_free(*(Cfire_Data **) ptr);
        free(ptr);
    }
}

#endif // NOT CFIRE_DECLARATION_ONLY

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

/* ^^^ End of loading values ^^^ */

#endif // CFIRE_LIBRARY_H
