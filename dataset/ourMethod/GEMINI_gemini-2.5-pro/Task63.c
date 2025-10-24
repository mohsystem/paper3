#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// --- BEGIN: Self-contained cJSON library ---
// This is an amalgamation of cJSON.h and cJSON.c for a single-file solution.
// cJSON v1.7.17 (https://github.com/DaveGamble/cJSON)
// Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall
#else
#define CJSON_CDECL
#define CJSON_STDCALL
#endif

#include <stddef.h>
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
      void *(CJSON_CDECL *malloc_fn)(size_t sz);
      void (CJSON_CDECL *free_fn)(void *ptr);
} cJSON_Hooks;

extern void CJSON_CDECL cJSON_InitHooks(cJSON_Hooks* hooks);
extern cJSON * CJSON_CDECL cJSON_Parse(const char *value);
extern char  * CJSON_CDECL cJSON_Print(const cJSON *item);
extern void    CJSON_CDECL cJSON_Delete(cJSON *c);
extern cJSON *cJSON_ParseWithLength(const char *value, size_t buffer_length);

#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7)

#ifdef __cplusplus
}
#endif
#endif

// --- cJSON.c implementation ---
static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;

static unsigned char cJSON_hex_to_int(unsigned char h)
{
    if (h >= '0' && h <= '9') return h - '0';
    if (h >= 'A' && h <= 'F') return h - 'A' + 10;
    if (h >= 'a' && h <= 'f') return h - 'a' + 10;
    return 0;
}

static const unsigned char* parse_value(cJSON *item, const unsigned char *value);

static cJSON *cJSON_New_Item(void)
{
    cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
    if (node) memset(node, 0, sizeof(cJSON));
    return node;
}

void cJSON_Delete(cJSON *c)
{
    cJSON *next;
    while (c)
    {
        next = c->next;
        if (!(c->type & cJSON_Raw) && c->child) cJSON_Delete(c->child);
        if (!(c->type & cJSON_Raw) && c->valuestring) cJSON_free(c->valuestring);
        if (!(c->type & cJSON_Raw) && c->string) cJSON_free(c->string);
        cJSON_free(c);
        c = next;
    }
}

static const unsigned char* skip(const unsigned char *in)
{
    if (in == NULL) return NULL;
    while (*in && *in <= 32) in++;
    return in;
}

static int pow2gt (int x) { --x; x|=x>>1; x|=x>>2; x|=x>>4; x|=x>>8; x|=x>>16; return x+1; }

typedef struct {char *buffer; int length; int offset; } printbuffer;

static char* ensure(printbuffer *p, int needed)
{
    char *newbuffer;int newsize;
    if (!p || !p->buffer) return 0;
    needed+=p->offset;
    if (needed<=p->length) return p->buffer+p->offset;

    newsize = pow2gt(needed);
    newbuffer = (char*)cJSON_malloc(newsize);
    if (!newbuffer) { cJSON_free(p->buffer); p->length=0, p->buffer=0; return 0; }
    if (newbuffer) memcpy(newbuffer,p->buffer,p->length);
    cJSON_free(p->buffer);
    p->length=newsize;
    p->buffer=newbuffer;
    return newbuffer+p->offset;
}

static int update(printbuffer *p)
{
    char *str;
    if (!p || !p->buffer) return 0;
    str=p->buffer+p->offset;
    return p->offset+strlen(str);
}

static char *print_string_ptr(const char *str, printbuffer *p)
{
    const char *ptr;char *ptr2,*out;int len=0,flag=0;unsigned char token;
    
    if (!str)
    {
        if (p) out=ensure(p,3);
        else out=(char*)cJSON_malloc(3);
        if (!out) return 0;
        strcpy(out,"\"\"");
        return out;
    }
    
    for (ptr=str;*ptr;ptr++) flag|=((*ptr>0 && *ptr<32)||(*ptr=='\"')||(*ptr=='\\'))?1:0;
    if (!flag)
    {
        len=ptr-str;
        if (p) out=ensure(p,len+3);
        else out=(char*)cJSON_malloc(len+3);
        if (!out) return 0;
        ptr2=out;*ptr2++='\"';
        strcpy(ptr2,str);
        ptr2[len]='\"';
        ptr2[len+1]=0;
        return out;
    }
    
    ptr=str;while ((token=*ptr) && ++len) {if (strchr("\"\\\b\f\n\r\t",token)) len++; else if (token<32) len+=5;}
    
    if (p) out=ensure(p,len+3);
    else out=(char*)cJSON_malloc(len+3);
    if (!out) return 0;

    ptr2=out;ptr=str;
    *ptr2++='\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
        else
        {
            *ptr2++='\\';
            switch (token=*ptr++)
            {
                case '\\':  *ptr2++='\\';   break;
                case '\"':  *ptr2++='\"';   break;
                case '\b':  *ptr2++='b';    break;
                case '\f':  *ptr2++='f';    break;
                case '\n':  *ptr2++='n';    break;
                case '\r':  *ptr2++='r';    break;
                case '\t':  *ptr2++='t';    break;
                default: sprintf(ptr2,"u%04x",token);ptr2+=5;   break;
            }
        }
    }
    *ptr2++='\"';*ptr2++=0;
    return out;
}

static char *print_value(cJSON *item,int depth,int fmt,printbuffer *p)
{
    char *out=0;
    if (!item) return 0;
    if(p)
    {
        switch ((item->type)&255)
        {
            case cJSON_NULL:    {out=ensure(p,5);   if (out) strcpy(out,"null");    break;}
            case cJSON_False:   {out=ensure(p,6);   if (out) strcpy(out,"false");   break;}
            case cJSON_True:    {out=ensure(p,5);   if (out) strcpy(out,"true");    break;}
            case cJSON_String:  out=print_string_ptr(item->valuestring,p);break;
        }
    }
    else
    {
        switch ((item->type)&255)
        {
            case cJSON_NULL:    out = strdup("null"); break;
            case cJSON_False:   out = strdup("false"); break;
            case cJSON_True:    out = strdup("true"); break;
            case cJSON_String:  out=print_string_ptr(item->valuestring,0);break;
        }
    }
    return out;
}


char *cJSON_Print(cJSON *item) {return print_value(item,0,1,0);}

static const unsigned char* parse_string(cJSON *item,const unsigned char *str)
{
    const unsigned char *ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
    if (*str!='\"') return 0;
    
    while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;
    
    out=(char*)cJSON_malloc(len+1);
    if (!out) return 0;
    
    ptr=str+1;ptr2=out;
    while (*ptr!='\"' && *ptr)
    {
        if (*ptr!='\\') *ptr2++=*ptr++;
        else
        {
            ptr++;
            switch (*ptr)
            {
                case 'b': *ptr2++='\b'; break;
                case 'f': *ptr2++='\f'; break;
                case 'n': *ptr2++='\n'; break;
                case 'r': *ptr2++='\r'; break;
                case 't': *ptr2++='\t'; break;
                case 'u':
                    uc = (cJSON_hex_to_int(ptr[1]) << 12) + (cJSON_hex_to_int(ptr[2]) << 8) + (cJSON_hex_to_int(ptr[3]) << 4) + cJSON_hex_to_int(ptr[4]);
                    ptr+=4;
                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)   break;
                    if (uc >= 0xD800 && uc <= 0xDBFF)
                    {
                        if (ptr[1]!='\\' || ptr[2]!='u')    break;
                        uc2 = (cJSON_hex_to_int(ptr[3]) << 12) + (cJSON_hex_to_int(ptr[4]) << 8) + (cJSON_hex_to_int(ptr[5]) << 4) + cJSON_hex_to_int(ptr[6]);
                        ptr+=6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF)       break;
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }
                    if (uc < 0x80) *ptr2++ = uc;
                    else if (uc < 0x800)
                    {
                        *ptr2++ = 0xC0 | (uc >> 6);
                        *ptr2++ = 0x80 | (uc & 0x3F);
                    }
                    else if (uc < 0x10000)
                    {
                        *ptr2++ = 0xE0 | (uc >> 12);
                        *ptr2++ = 0x80 | ((uc >> 6) & 0x3F);
                        *ptr2++ = 0x80 | (uc & 0x3F);
                    }
                    else
                    {
                        *ptr2++ = 0xF0 | (uc >> 18);
                        *ptr2++ = 0x80 | ((uc >> 12) & 0x3F);
                        *ptr2++ = 0x80 | ((uc >> 6) & 0x3F);
                        *ptr2++ = 0x80 | (uc & 0x3F);
                    }
                    break;
                default:  *ptr2++=*ptr; break;
            }
            ptr++;
        }
    }
    *ptr2=0;
    if (*ptr=='\"') ptr++;
    item->valuestring=out;
    item->type=cJSON_String;
    return ptr;
}

static const unsigned char* parse_number(cJSON *item,const unsigned char *num)
{
    double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;

    if (*num=='-') sign=-1,num++;
    if (*num=='0') num++;
    if (*num>='1' && *num<='9') do n=(n*10.0)+(*num++ -'0'); while (*num>='0' && *num<='9');
    if (*num=='.' && num[1]>='0' && num[1]<='9') {num++;      do n=(n*10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');}
    if (*num=='e' || *num=='E')
    {   num++;if (*num=='+') num++; else if (*num=='-') signsubscale=-1,num++;
        while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');
    }

    n=sign*n*pow(10.0,(scale+subscale*signsubscale));
    
    item->valuedouble=n;
    item->valueint=(int)n;
    item->type=cJSON_Number;
    return num;
}

static const unsigned char* parse_object(cJSON *item,const unsigned char *value)
{
    cJSON *child;
    if (*value!='{')    return 0;
    
    item->type=cJSON_Object;
    value=skip(value+1);
    if (*value=='}') return value+1;
    
    item->child=child=cJSON_New_Item();
    if (!item->child) return 0;
    value=skip(parse_string(child,skip(value)));
    if (!value) return 0;
    child->string=child->valuestring;child->valuestring=0;
    if (*value!=':') return 0;
    value=skip(parse_value(child,skip(value+1)));
    if (!value) return 0;
    
    while (*value==',')
    {
        cJSON *new_item;
        if (!(new_item=cJSON_New_Item()))   return 0;
        child->next=new_item;new_item->prev=child;child=new_item;
        value=skip(parse_string(child,skip(value+1)));
        if (!value) return 0;
        child->string=child->valuestring;child->valuestring=0;
        if (*value!=':') return 0;
        value=skip(parse_value(child,skip(value+1)));
        if (!value) return 0;
    }
    
    if (*value=='}') return value+1;
    return 0;
}

static const unsigned char* parse_value(cJSON *item, const unsigned char *value)
{
    if (!value) return 0;
    if (!strncmp((const char*)value,"null",4)) { item->type=cJSON_NULL;  return value+4; }
    if (!strncmp((const char*)value,"false",5)){ item->type=cJSON_False; return value+5; }
    if (!strncmp((const char*)value,"true",4)) { item->type=cJSON_True; item->valueint=1; return value+4; }
    if (*value=='\"') { return parse_string(item,value); }
    if (*value=='-' || (*value>='0' && *value<='9')) { return parse_number(item,value); }
    if (*value=='{') { return parse_object(item,value); }
    
    return 0;
}

cJSON *cJSON_ParseWithLength(const char *value, size_t buffer_length)
{
    const unsigned char* end = NULL;
    cJSON *c = cJSON_New_Item();
    if (!c) return NULL;
    
    end = parse_value(c, (const unsigned char*)value);
    if (!end) { cJSON_Delete(c); return NULL; }
    
    return c;
}

cJSON *cJSON_Parse(const char *value)
{
    return cJSON_ParseWithLength(value, strlen(value));
}

// --- END: Self-contained cJSON library ---


// --- BEGIN: Self-contained Base64 decoder ---
// Source: https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/
// with modifications for validation and memory management.
static const int b64_char_to_val[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

unsigned char *b64_decode(const char *input, size_t *output_len) {
    size_t input_len = strlen(input);
    if (input_len % 4 != 0) return NULL;

    *output_len = input_len / 4 * 3;
    if (input[input_len - 1] == '=') (*output_len)--;
    if (input[input_len - 2] == '=') (*output_len)--;
    
    unsigned char *decoded = (unsigned char *)malloc(*output_len + 1);
    if (decoded == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_len; i += 4, j += 3) {
        int v[4];
        for(int k=0; k<4; k++) {
            v[k] = b64_char_to_val[(unsigned char)input[i+k]];
            if (v[k] == -1 && input[i+k] != '=') { // Invalid char check
                free(decoded);
                return NULL;
            }
        }
        
        decoded[j] = (unsigned char)((v[0] << 2) + ((v[1] & 0x30) >> 4));
        if (j + 1 < *output_len) {
            decoded[j + 1] = (unsigned char)(((v[1] & 0x0F) << 4) + ((v[2] & 0x3C) >> 2));
        }
        if (j + 2 < *output_len) {
            decoded[j + 2] = (unsigned char)(((v[2] & 0x03) << 6) + v[3]);
        }
    }
    decoded[*output_len] = '\0';
    return decoded;
}
// --- END: Self-contained Base64 decoder ---

/**
 * @brief Processes a request by decoding and deserializing the raw data.
 * @param rawData A null-terminated C string containing UTF-8, Base64 encoded JSON data.
 * @return A pointer to a cJSON object, or NULL on failure. The caller must free the object with cJSON_Delete.
 */
cJSON* processRequest(const char* rawData) {
    if (rawData == NULL) {
        return NULL;
    }

    // 1. & 2. Retrieve raw_data and decode from Base64
    size_t decoded_len = 0;
    unsigned char* decoded_data = b64_decode(rawData, &decoded_len);
    if (decoded_data == NULL) {
        return NULL; // Base64 decoding failed
    }

    // 3. Deserialize the decoded data (JSON)
    cJSON* data = cJSON_ParseWithLength((const char*)decoded_data, decoded_len);
    
    // Free the buffer used for decoded data
    free(decoded_data);

    // cJSON_Parse returns NULL on failure.
    if (data == NULL) {
        return NULL;
    }

    // 4. & 5. Return the stored (deserialized) data
    return data;
}

int main() {
    const char* testCases[] = {
        // Test Case 1: Valid input
        "eyJ1c2VyIjogInRlc3QiLCAiaWQiOiAxMjMsICJhY3RpdmUiOiB0cnVlfQ==",
        // Test Case 2: Another valid input
        "eyJoZWxsbyI6ICJ3b3JsZCJ9",
        // Test Case 3: Invalid Base64
        "this-is-not-base64",
        // Test Case 4: Valid Base64, but not JSON
        "aGVsbG8gd29ybGQ=", // "hello world"
        // Test Case 5: Empty input
        ""
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input: \"%s\"\n", testCases[i]);
        cJSON* result = processRequest(testCases[i]);
        if (result != NULL) {
            char* json_string = cJSON_Print(result);
            printf("Output: %s\n", json_string);
            free(json_string);
            cJSON_Delete(result);
        } else {
            printf("Output: Processing failed (NULL)\n");
        }
        printf("--------------------\n");
    }

    return 0;
}