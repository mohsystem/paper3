/*
 * This single C file includes the cJSON library followed by the main program logic.
 * cJSON is a lightweight and robust JSON parser. By embedding it, we ensure there
 * are no external dependencies besides the standard C library.
 *
 * Compile command example:
 * gcc -o task62 Task62.c -lm
 *
 * The code demonstrates secure practices for C:
 * 1. Using a trusted library (cJSON) for parsing complex formats.
 * 2. Checking for NULL pointers returned by library functions.
 * 3. Proper memory management: freeing all allocated memory from cJSON
 *    (`cJSON_Delete` for parsed objects, `free` for printed strings).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 ===============================================================================
 I. BEGIN EMBEDDED cJSON LIBRARY (Version 1.7.17)
 ===============================================================================
 cJSON Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
 cJSON is licensed under the MIT License.
 ===============================================================================
*/

/* cJSON.h */
#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#include <stddef.h>

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
typedef struct cJSON
{
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;

typedef struct cJSON_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

typedef int cJSON_bool;

/* Supply cJSON with custom memory functions when you are not linking to malloc/free()
 * Alternatively you can define CJSON_MALLOC and CJSON_FREE directly.
 * It is not possible to call cJSON_InitHooks() twice. */
extern void cJSON_InitHooks(cJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible for freeing returned cJSON objects. */
extern cJSON *cJSON_New_Item(void);
extern cJSON *cJSON_CreateNull(void);
extern cJSON *cJSON_CreateTrue(void);
extern cJSON *cJSON_CreateFalse(void);
extern cJSON *cJSON_CreateBool(cJSON_bool boolean);
extern cJSON *cJSON_CreateNumber(double num);
extern cJSON *cJSON_CreateString(const char *string);
extern cJSON *cJSON_CreateRaw(const char *raw);
extern cJSON *cJSON_CreateArray(void);
extern cJSON *cJSON_CreateObject(void);
extern cJSON *cJSON_CreateStringReference(const char *string);
extern cJSON *cJSON_CreateObjectReference(const cJSON *child);
extern cJSON *cJSON_CreateArrayReference(const cJSON *child);
extern cJSON *cJSON_Duplicate(const cJSON *item, cJSON_bool recurse);
extern void cJSON_Delete(cJSON *c);

/* Parse a lang-format string into a cJSON object. */
extern cJSON *cJSON_Parse(const char *value);
extern cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);

/* Render a cJSON entity to text for transfer/storage. */
extern char *cJSON_Print(const cJSON *item);
extern char *cJSON_PrintUnformatted(const cJSON *item);
extern char *cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);
extern cJSON_bool cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);

#define cJSON_GetErrorPtr() cJSON_GetErrorPtr()
extern const char *cJSON_GetErrorPtr(void);

#define cJSON_IsInvalid(item) ((item) ? (item->type & 0xFF) == cJSON_Invalid : 1)
#define cJSON_IsFalse(item) ((item) ? (item->type & 0xFF) == cJSON_False : 0)
#define cJSON_IsTrue(item) ((item) ? (item->type & 0xFF) == cJSON_True : 0)
#define cJSON_IsBool(item) (cJSON_IsFalse(item) || cJSON_IsTrue(item))
#define cJSON_IsNull(item) ((item) ? (item->type & 0xFF) == cJSON_NULL : 0)
#define cJSON_IsNumber(item) ((item) ? (item->type & 0xFF) == cJSON_Number : 0)
#define cJSON_IsString(item) ((item) ? (item->type & 0xFF) == cJSON_String : 0)
#define cJSON_IsArray(item) ((item) ? (item->type & 0xFF) == cJSON_Array : 0)
#define cJSON_IsObject(item) ((item) ? (item->type & 0xFF) == cJSON_Object : 0)
#define cJSON_IsRaw(item) ((item) ? (item->type & 0xFF) == cJSON_Raw : 0)

#ifdef __cplusplus
}
#endif

#endif

/* cJSON.c */
#include <math.h>
#include <limits.h>
#include <ctype.h>

#ifndef CJSON_VERSION_MAJOR
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 17
#endif

static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;
static const char *global_ep = NULL;

const char *cJSON_GetErrorPtr(void)
{
    return global_ep;
}

static int cJSON_strcasecmp(const char *s1, const char *s2)
{
    if (!s1)
    {
        return (s1 == s2) ? 0 : 1;
    }
    if (!s2)
    {
        return 1;
    }
    for(; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2)
    {
        if (*s1 == 0)
        {
            return 0;
        }
    }
    return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

static char* cJSON_strdup(const char* str)
{
    size_t len;
    char* copy;
    len = strlen(str) + 1;
    if (!(copy = (char*)cJSON_malloc(len)))
    {
        return NULL;
    }
    memcpy(copy, str, len);
    return copy;
}

void cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (!hooks)
    {
        cJSON_malloc = malloc;
        cJSON_free = free;
        return;
    }
    cJSON_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
    cJSON_free = (hooks->free_fn) ? hooks->free_fn : free;
}

cJSON *cJSON_New_Item(void)
{
    cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
    if (node)
    {
        memset(node, '\0', sizeof(cJSON));
    }
    return node;
}

void cJSON_Delete(cJSON *c)
{
    cJSON *next;
    while (c)
    {
        next = c->next;
        if (!(c->type & cJSON_IsReference) && c->child)
        {
            cJSON_Delete(c->child);
        }
        if (!(c->type & cJSON_IsReference) && c->valuestring)
        {
            cJSON_free(c->valuestring);
        }
        if (!(c->type & cJSON_StringIsConst) && c->string)
        {
            cJSON_free(c->string);
        }
        cJSON_free(c);
        c = next;
    }
}

static const char *parse_value(cJSON *item, const char *value);
static char *print_value(const cJSON *item, int depth, int fmt, void* p);
static const char *parse_number(cJSON *item, const char *num);

cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    const char *end = NULL;
    cJSON *c = cJSON_New_Item();
    if (!c)
    {
        return NULL;
    }
    global_ep = NULL;
    end = parse_value(c, value);
    if (!end)
    {
        cJSON_Delete(c);
        return NULL;
    }
    if (require_null_terminated)
    {
        end = value + strlen(value);
        while (end > value && isspace((unsigned char)end[-1])) end--;
        if (*end)
        {
            cJSON_Delete(c);
            global_ep = end;
            return NULL;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = end;
    }
    return c;
}

cJSON *cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 1);
}

#define cjson_min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char* cJSON_hex_to_uint4(const unsigned char * const hex)
{
    static unsigned char first_byte[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
        0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    unsigned char *output = (unsigned char*)cJSON_malloc(1);
    *output = first_byte[hex[0]] << 4 | first_byte[hex[1]];
    return output;
}

static const char *parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc, uc2;
    if (*str != '\"')
    {
        global_ep = str;
        return NULL;
    }
    while (*ptr != '\"' && *ptr && ++len)
    {
        if (*ptr++ == '\\')
        {
            ptr++;
        }
    }
    out = (char*)cJSON_malloc(len + 1);
    if (!out)
    {
        return NULL;
    }
    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }
        else
        {
            ptr++;
            switch (*ptr)
            {
                case 'b':
                    *ptr2++ = '\b';
                    break;
                case 'f':
                    *ptr2++ = '\f';
                    break;
                case 'n':
                    *ptr2++ = '\n';
                    break;
                case 'r':
                    *ptr2++ = '\r';
                    break;
                case 't':
                    *ptr2++ = '\t';
                    break;
                case 'u':
                    sscanf(ptr + 1, "%4x", &uc);
                    ptr += 4;
                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
                    {
                        break;
                    }
                    if (uc >= 0xD800 && uc <= 0xDBFF)
                    {
                        if (ptr[1] != '\\' || ptr[2] != 'u')
                        {
                            break;
                        }
                        sscanf(ptr + 3, "%4x", &uc2);
                        ptr += 6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF)
                        {
                            break;
                        }
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }
                    len = 4;
                    if (uc < 0x80)
                    {
                        len = 1;
                    }
                    else if (uc < 0x800)
                    {
                        len = 2;
                    }
                    else if (uc < 0x10000)
                    {
                        len = 3;
                    }
                    ptr2 += len;
                    switch (len)
                    {
                        case 4:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 3:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1:
                            *--ptr2 = (uc | ((len > 1) ? ((0x1E0 / (len+1)) << 2) : 0));
                    }
                    ptr2 += len;
                    break;
                default:
                    *ptr2++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"')
    {
        ptr++;
    }
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

static const char *parse_value(cJSON *item, const char *value)
{
    if (!value)
    {
        return NULL;
    }
    if (!strncmp(value, "null", 4))
    {
        item->type = cJSON_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5))
    {
        item->type = cJSON_False;
        return value + 5;
    }
    if (!strncmp(value, "true", 4))
    {
        item->type = cJSON_True;
        item->valueint = 1;
        return value + 4;
    }
    if (*value == '\"')
    {
        return parse_string(item, value);
    }
    if (*value == '-' || (*value >= '0' && *value <= '9'))
    {
        return parse_number(item, value);
    }
    if (*value == '[')
    {
        cJSON *child;
        item->type = cJSON_Array;
        value++;
        if (*value == ']')
        {
            return value + 1;
        }
        item->child = child = cJSON_New_Item();
        if (!item->child)
        {
            return NULL;
        }
        value = parse_value(child, value);
        if (!value)
        {
            return NULL;
        }
        while (*value == ',')
        {
            cJSON *new_item;
            value++;
            if (!(new_item = cJSON_New_Item()))
            {
                return NULL;
            }
            child->next = new_item;
            new_item->prev = child;
            child = new_item;
            value = parse_value(child, value);
            if (!value)
            {
                return NULL;
            }
        }
        if (*value == ']')
        {
            return value + 1;
        }
        global_ep = value;
        return NULL;
    }
    if (*value == '{')
    {
        cJSON *child;
        item->type = cJSON_Object;
        value++;
        if (*value == '}')
        {
            return value + 1;
        }
        item->child = child = cJSON_New_Item();
        if (!item->child)
        {
            return NULL;
        }
        value = parse_string(child, value);
        if (!value)
        {
            return NULL;
        }
        child->string = child->valuestring;
        child->valuestring = 0;
        if (*value != ':')
        {
            global_ep = value;
            return NULL;
        }
        value++;
        value = parse_value(child, value);
        if (!value)
        {
            return NULL;
        }
        while (*value == ',')
        {
            cJSON *new_item;
            value++;
            if (!(new_item = cJSON_New_Item()))
            {
                return NULL;
            }
            child->next = new_item;
            new_item->prev = child;
            child = new_item;
            value = parse_string(child, value);
            if (!value)
            {
                return NULL;
            }
            child->string = child->valuestring;
            child->valuestring = 0;
            if (*value != ':')
            {
                global_ep = value;
                return NULL;
            }
            value++;
            value = parse_value(child, value);
            if (!value)
            {
                return NULL;
            }
        }
        if (*value == '}')
        {
            return value + 1;
        }
        global_ep = value;
        return NULL;
    }
    global_ep = value;
    return NULL;
}

static const char *parse_number(cJSON *item, const char *num)
{
    double n = 0, sign = 1, scale = 0;
    int subscale = 0, signsubscale = 1;
    if (*num == '-')
    {
        sign = -1;
        num++;
    }
    if (*num == '0')
    {
        num++;
    }
    if (*num >= '1' && *num <= '9')
    {
        do
        {
            n = (n * 10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }
    if (*num == '.' && num[1] >= '0' && num[1] <= '9')
    {
        num++;
        do
        {
            n = (n * 10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }
    if (*num == 'e' || *num == 'E')
    {
        num++;
        if (*num == '+')
        {
            num++;
        }
        else if (*num == '-')
        {
            signsubscale = -1;
            num++;
        }
        while (*num >= '0' && *num <= '9')
        {
            subscale = (subscale * 10) + (*num++ - '0');
        }
    }
    n = sign * n * pow(10.0, (scale + subscale * signsubscale));
    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;
    return num;
}

/* Render a value to text. */
static char *print_value(const cJSON *item, int depth, int fmt, void* p)
{
    char *out = 0;
    if (!item)
    {
        return 0;
    }
    switch ((item->type) & 255)
    {
        case cJSON_NULL:
            out = cJSON_strdup("null");
            break;
        case cJSON_False:
            out = cJSON_strdup("false");
            break;
        case cJSON_True:
            out = cJSON_strdup("true");
            break;
        case cJSON_Number:
        {
            char buffer[26];
            double d = item->valuedouble;
            if (fabs(((double)item->valueint) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
            {
                sprintf(buffer, "%d", item->valueint);
            }
            else
            {
                sprintf(buffer, "%1.15g", d);
            }
            out = cJSON_strdup(buffer);
            break;
        }
        case cJSON_String:
        {
            char *str = item->valuestring, *ptr, *ptr2, *out_ptr;
            size_t len = 0, flag = 0;
            if (!str)
            {
                out = cJSON_strdup("\"\"");
                break;
            }
            ptr = str;
            while ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\' && ++len)
            {
                ptr++;
            }
            if (!*ptr)
            {
                len = strlen(str);
                out = (char*)cJSON_malloc(len + 3);
                if (!out)
                {
                    return 0;
                }
                ptr2 = out;
                *ptr2++ = '\"';
                strcpy(ptr2, str);
                ptr2[len] = '\"';
                ptr2[len + 1] = 0;
                return out;
            }
            out = (char*)cJSON_malloc(strlen(str) + 6);
            if (!out)
            {
                return 0;
            }
            out_ptr = out;
            *out_ptr++ = '\"';
            ptr = str;
            while (*ptr)
            {
                if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
                {
                    *out_ptr++ = *ptr++;
                }
                else
                {
                    *out_ptr++ = '\\';
                    switch (*ptr++)
                    {
                        case '\\':
                            *out_ptr++ = '\\';
                            break;
                        case '\"':
                            *out_ptr++ = '\"';
                            break;
                        case '\b':
                            *out_ptr++ = 'b';
                            break;
                        case '\f':
                            *out_ptr++ = 'f';
                            break;
                        case '\n':
                            *out_ptr++ = 'n';
                            break;
                        case '\r':
                            *out_ptr++ = 'r';
                            break;
                        case '\t':
                            *out_ptr++ = 't';
                            break;
                        default:
                            sprintf(out_ptr, "u%04x", *ptr-1);
                            out_ptr += 5;
                            break;
                    }
                }
            }
            *out_ptr++ = '\"';
            *out_ptr = 0;
            break;
        }
        case cJSON_Array:
        {
            char **entries;
            int i = 0, numentries = 0;
            cJSON *child = item->child;
            size_t len = 0;
            char *out2;
            while (child)
            {
                numentries++;
                child = child->next;
            }
            if (!numentries)
            {
                out = cJSON_strdup("[]");
                break;
            }
            entries = (char**)cJSON_malloc(numentries * sizeof(char*));
            if (!entries)
            {
                return 0;
            }
            memset(entries, 0, numentries * sizeof(char*));
            child = item->child;
            while (child)
            {
                entries[i++] = print_value(child, depth + 1, fmt, 0);
                if (!entries[i-1])
                {
                    for (i = i - 2; i >= 0; i--) cJSON_free(entries[i]);
                    cJSON_free(entries);
                    return 0;
                }
                len += strlen(entries[i-1]) + 2;
                child = child->next;
            }
            out = (char*)cJSON_malloc(len);
            if (!out)
            {
                for (i=0; i<numentries; i++) cJSON_free(entries[i]);
                cJSON_free(entries);
                return 0;
            }
            *out = '[';
            out2 = out + 1;
            for (i=0; i<numentries; i++)
            {
                strcpy(out2, entries[i]);
                out2 += strlen(entries[i]);
                if (i != numentries - 1)
                {
                    *out2++ = ',';
                    if(fmt) *out2++ = ' ';
                }
                cJSON_free(entries[i]);
            }
            cJSON_free(entries);
            *out2++ = ']';
            *out2 = 0;
            break;
        }
        case cJSON_Object:
        {
            char **entries = 0, **names = 0;
            int i = 0, j, numentries = 0;
            cJSON *child = item->child;
            size_t len = 0;
            char *out2;
            while (child)
            {
                numentries++;
                child = child->next;
            }
            if (!numentries)
            {
                out = cJSON_strdup("{}");
                break;
            }
            entries = (char**)cJSON_malloc(numentries * sizeof(char*));
            if (!entries) return 0;
            names = (char**)cJSON_malloc(numentries * sizeof(char*));
            if (!names)
            {
                cJSON_free(entries);
                return 0;
            }
            memset(entries, 0, sizeof(char*) * numentries);
            memset(names, 0, sizeof(char*) * numentries);
            child = item->child;
            depth++;
            if (fmt) len += depth;
            while (child)
            {
                names[i] = print_string(child, 0);
                entries[i++] = print_value(child, depth, fmt, 0);
                if (!names[i-1] || !entries[i-1])
                {
                    for (j=0; j<i; j++) { cJSON_free(names[j]); cJSON_free(entries[j]); }
                    cJSON_free(names);
                    cJSON_free(entries);
                    return 0;
                }
                len += strlen(names[i-1]) + strlen(entries[i-1]) + 2 + (fmt ? 2 + depth : 0);
                child = child->next;
            }
            out = (char*)cJSON_malloc(len + 1);
            if (!out)
            {
                for (i=0; i<numentries; i++) { cJSON_free(names[i]); cJSON_free(entries[i]); }
                cJSON_free(names);
                cJSON_free(entries);
                return 0;
            }
            *out = '{';
            out2 = out + 1;
            if (fmt) *out2++ = '\n';
            for (i=0; i<numentries; i++)
            {
                if (fmt) for (j=0; j<depth; j++) *out2++ = '\t';
                strcpy(out2, names[i]);
                out2 += strlen(names[i]);
                *out2++ = ':';
                if (fmt) *out2++ = '\t';
                strcpy(out2, entries[i]);
                out2 += strlen(entries[i]);
                if (i != numentries - 1) *out2++ = ',';
                if (fmt) *out2++ = '\n';
                cJSON_free(names[i]);
                cJSON_free(entries[i]);
            }
            cJSON_free(names);
            cJSON_free(entries);
            if (fmt) for (i=0; i<depth - 1; i++) *out2++ = '\t';
            *out2++ = '}';
            *out2 = 0;
            break;
        }
    }
    return out;
}

char *cJSON_Print(const cJSON *item)
{
    return print_value(item, 0, 1, 0);
}

char *cJSON_PrintUnformatted(const cJSON *item)
{
    return print_value(item, 0, 0, 0);
}

/*
 ===============================================================================
 II. END EMBEDDED cJSON LIBRARY
 ===============================================================================
*/


/*
 ===============================================================================
 III. BEGIN TASK-SPECIFIC CODE
 ===============================================================================
*/

/**
 * Parses a JSON string and returns its root element as a cJSON object.
 *
 * IMPORTANT: The caller is responsible for freeing the returned cJSON object
 * using cJSON_Delete() to prevent memory leaks. This is a critical security
 * and stability measure.
 *
 * @param json_string The constant JSON string to parse.
 * @return A pointer to the root cJSON structure if parsing is successful,
 *         or NULL if an error occurs (e.g., malformed JSON, out of memory).
 */
cJSON* getJsonRoot(const char * const json_string) {
    if (json_string == NULL) {
        fprintf(stderr, "Error: Input JSON string is NULL.\n");
        return NULL;
    }

    // cJSON_Parse is designed to be safe against malformed input.
    // It returns NULL on failure and sets an internal error pointer.
    cJSON *root = cJSON_Parse(json_string);
    
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            // Log the position of the error for debugging purposes.
            fprintf(stderr, "Error parsing JSON near: %s\n", error_ptr);
        } else {
            fprintf(stderr, "Error parsing JSON: Unknown error (possibly out of memory).\n");
        }
    }
    return root;
}

/**
 * Helper function to get a string representation of a cJSON item's type.
 * @param item The cJSON item to inspect.
 * @return A constant string describing the item's type.
 */
const char* getElementType(const cJSON * const item) {
    if (cJSON_IsObject(item)) return "Object";
    if (cJSON_IsArray(item)) return "Array";
    if (cJSON_IsString(item)) return "String";
    if (cJSON_IsNumber(item)) return "Number";
    if (cJSON_IsBool(item)) return "Boolean";
    if (cJSON_IsNull(item)) return "Null";
    return "Unknown";
}


int main() {
    const char* testCases[] = {
        // 1. Valid JSON object
        "{\"name\": \"John Doe\", \"age\": 30, \"isStudent\": false, \"courses\": [\"Math\", \"Science\"]}",
        // 2. Valid JSON array
        "[1, \"test\", true, null, {\"key\": \"value\"}]",
        // 3. Valid JSON primitive (string)
        "\"Hello, Secure World!\"",
        // 4. Valid JSON primitive (number)
        "123.456",
        // 5. Invalid JSON string (malformed)
        "{\"name\": \"Jane Doe\", \"age\":}"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", testCases[i]);
        
        cJSON *rootElement = getJsonRoot(testCases[i]);
        
        // Securely handle the case where parsing fails.
        if (rootElement != NULL) {
            // cJSON_Print allocates a new string that must be freed by the caller.
            char *printed_json = cJSON_Print(rootElement);
            if (printed_json != NULL) {
                printf("Root Element Type: %s\n", getElementType(rootElement));
                printf("Parsed Output: %s\n", printed_json);
                // SECURITY: Free the memory allocated by cJSON_Print.
                free(printed_json);
            } else {
                fprintf(stderr, "Failed to print parsed JSON (likely out of memory).\n");
            }
            
            // SECURITY: Crucial step to prevent memory leaks. The object returned
            // by getJsonRoot (and cJSON_Parse) must be deleted.
            cJSON_Delete(rootElement);
        } else {
            printf("Failed to parse JSON.\n");
        }
        printf("\n");
    }

    return 0;
}