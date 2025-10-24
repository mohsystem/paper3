#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
Step 1: Problem understanding
- Provide a secure, JNDI-like in-memory registry for strings with bind/rebind/lookup.
Step 2: Security
- No network, validate names (allowed chars, length), restrict values to bounded size.
*/

#define MAX_NAME_LEN 256
#define MAX_VALUE_LEN 8192

typedef struct {
    char* name;
    char* value;
} Entry;

typedef struct {
    Entry* entries;
    size_t count;
    size_t capacity;
} Context;

static int is_allowed_char(char c) {
    return isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.' || c == '/';
}

static int valid_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > MAX_NAME_LEN) return 0;
    if (name[0] == '/' || name[len-1] == '/') return 0;
    for (size_t i = 0; i < len; ++i) {
        if (!is_allowed_char(name[i])) return 0;
        if (name[i] == '/' && i + 1 < len && name[i+1] == '/') return 0;
    }
    return 1;
}

static void ctx_init(Context* ctx) {
    ctx->entries = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
}

static void ctx_free(Context* ctx) {
    if (!ctx) return;
    for (size_t i = 0; i < ctx->count; ++i) {
        free(ctx->entries[i].name);
        free(ctx->entries[i].value);
    }
    free(ctx->entries);
    ctx->entries = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
}

static ssize_t ctx_index_of(Context* ctx, const char* name) {
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp(ctx->entries[i].name, name) == 0) return (ssize_t)i;
    }
    return -1;
}

static int ctx_ensure_capacity(Context* ctx, size_t needed) {
    if (ctx->capacity >= needed) return 0;
    size_t newcap = ctx->capacity == 0 ? 8 : ctx->capacity * 2;
    while (newcap < needed) newcap *= 2;
    Entry* n = (Entry*)realloc(ctx->entries, newcap * sizeof(Entry));
    if (!n) return -1;
    ctx->entries = n;
    ctx->capacity = newcap;
    return 0;
}

static int bind(Context* ctx, const char* name, const char* value) {
    if (!valid_name(name)) return -2; // invalid name
    if (!value) return -3; // null value
    size_t vlen = strlen(value);
    if (vlen > MAX_VALUE_LEN) return -4; // too large
    if (ctx_index_of(ctx, name) >= 0) return -5; // already bound
    if (ctx_ensure_capacity(ctx, ctx->count + 1) != 0) return -6; // oom
    char* n = (char*)malloc(strlen(name) + 1);
    char* v = (char*)malloc(vlen + 1);
    if (!n || !v) { free(n); free(v); return -6; }
    strcpy(n, name);
    memcpy(v, value, vlen + 1);
    ctx->entries[ctx->count].name = n;
    ctx->entries[ctx->count].value = v;
    ctx->count++;
    return 0;
}

static int rebind(Context* ctx, const char* name, const char* value) {
    if (!valid_name(name)) return -2;
    if (!value) return -3;
    size_t vlen = strlen(value);
    if (vlen > MAX_VALUE_LEN) return -4;
    ssize_t idx = ctx_index_of(ctx, name);
    if (idx >= 0) {
        char* v = (char*)malloc(vlen + 1);
        if (!v) return -6;
        memcpy(v, value, vlen + 1);
        free(ctx->entries[idx].value);
        ctx->entries[idx].value = v;
        return 0;
    } else {
        return bind(ctx, name, value);
    }
}

static int lookup(Context* ctx, const char* name, const char** outValue) {
    if (!valid_name(name)) return -2;
    ssize_t idx = ctx_index_of(ctx, name);
    if (idx < 0) return -1; // not found
    *outValue = ctx->entries[idx].value;
    return 0;
}

// Returns a newly allocated string with "name=value\n" lines for each lookup.
// Caller must free the returned buffer with free().
char* perform_lookups(const char** bindNames, const char** bindValues, size_t bindCount,
                      const char** lookupNames, size_t lookupCount) {
    Context ctx; ctx_init(&ctx);

    // Bind initial entries
    for (size_t i = 0; i < bindCount; ++i) {
        int rc = rebind(&ctx, bindNames[i], bindValues[i]);
        if (rc != 0) {
            // On invalid binding, build a small error string and return
            const char* msg = "ERROR: invalid binding encountered\n";
            char* out = (char*)malloc(strlen(msg) + 1);
            if (out) strcpy(out, msg);
            ctx_free(&ctx);
            return out;
        }
    }

    // Estimate output size
    size_t total = 0;
    for (size_t i = 0; i < lookupCount; ++i) {
        size_t nameLen = strlen(lookupNames[i]);
        total += nameLen + 1 + 12 + 1; // "name=value\n" with value maybe "<NOT_FOUND>" (11 chars)
        // If found, value could be larger; for safety, add up to min(len, MAX_VALUE_LEN)
        const char* val = NULL;
        int rc = lookup(&ctx, lookupNames[i], &val);
        if (rc == 0 && val) {
            size_t vlen = strlen(val);
            if (vlen > MAX_VALUE_LEN) vlen = MAX_VALUE_LEN;
            total += vlen;
        } else if (rc == -2) {
            total += strlen("<INVALID_NAME>");
        } else {
            total += strlen("<NOT_FOUND>");
        }
    }

    char* out = (char*)malloc(total + 1);
    if (!out) { ctx_free(&ctx); return NULL; }
    out[0] = '\0';

    for (size_t i = 0; i < lookupCount; ++i) {
        const char* name = lookupNames[i];
        strcat(out, name);
        strcat(out, "=");
        const char* val = NULL;
        int rc = lookup(&ctx, name, &val);
        if (rc == 0 && val) {
            strcat(out, val);
        } else if (rc == -2) {
            strcat(out, "<INVALID_NAME>");
        } else {
            strcat(out, "<NOT_FOUND>");
        }
        strcat(out, "\n");
    }

    ctx_free(&ctx);
    return out;
}

int main(void) {
    // Test 1
    const char* b1n[] = {"config/db/url", "feature/flagA"};
    const char* b1v[] = {"jdbc:h2:mem:test1", "true"};
    const char* l1[] = {"config/db/url", "feature/flagA", "missing/name"};
    char* r1 = perform_lookups(b1n, b1v, 2, l1, 3);
    printf("Test1:\n%s", r1); free(r1);

    // Test 2
    const char* b2n[] = {"service/endpoint"};
    const char* b2v[] = {"https://api.v2.example.com"};
    const char* l2[] = {"service/endpoint"};
    char* r2 = perform_lookups(b2n, b2v, 1, l2, 1);
    printf("Test2:\n%s", r2); free(r2);

    // Test 3
    const char* b3n[] = {"app/env/prod/log/level", "app/env/prod/threads"};
    const char* b3v[] = {"INFO", "16"};
    const char* l3[] = {"app/env/prod/log/level", "app/env/prod/threads"};
    char* r3 = perform_lookups(b3n, b3v, 2, l3, 2);
    printf("Test3:\n%s", r3); free(r3);

    // Test 4 (invalid binding)
    const char* b4n[] = {"/bad/leading/slash"};
    const char* b4v[] = {"x"};
    const char* l4[] = {"/bad/leading/slash", "also//bad"};
    char* r4 = perform_lookups(b4n, b4v, 1, l4, 2);
    printf("Test4:\n%s", r4); free(r4);

    // Test 5 (large value)
    char* big = (char*)malloc(10001);
    big[0] = '\0';
    for (int i = 0; i < 1000; ++i) strcat(big, "0123456789");
    const char* b5n[] = {"blob/data"};
    const char* b5v[] = {(const char*)big};
    const char* l5[] = {"blob/data", "not/present"};
    char* r5 = perform_lookups(b5n, b5v, 1, l5, 2);
    printf("Test5:\n%s", r5);
    free(r5);
    free(big);

    return 0;
}