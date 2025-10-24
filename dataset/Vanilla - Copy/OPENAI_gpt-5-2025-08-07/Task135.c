#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* session_id;
    char* token;
} Entry;

static Entry* store = NULL;
static size_t store_count = 0;
static size_t store_cap = 0;

static char* my_strdup(const char* s) {
    size_t len = strlen(s);
    char* d = (char*)malloc(len + 1);
    if (!d) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

static int get_random_bytes(unsigned char* buf, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(buf, 1, len, f);
    fclose(f);
    return r == len;
}

static char* base64url_encode(const unsigned char* data, size_t len) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t out_cap = ((len + 2) / 3) * 4 + 1;
    char* out = (char*)malloc(out_cap);
    if (!out) return NULL;

    size_t i = 0, o = 0;
    while (i + 3 <= len) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
        out[o++] = alphabet[(v >> 6) & 0x3F];
        out[o++] = alphabet[v & 0x3F];
        i += 3;
    }
    if (i + 1 == len) {
        unsigned int v = (data[i] << 16);
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
        // no padding, so omit last two
    } else if (i + 2 == len) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8);
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
        out[o++] = alphabet[(v >> 6) & 0x3F];
        // no padding, omit last one
    }
    out[o] = '\0';
    return out;
}

static int constant_time_equals(const char* a, const char* b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t lmin = la < lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < lmin; ++i) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static int find_entry_index(const char* session_id) {
    for (size_t i = 0; i < store_count; ++i) {
        if (strcmp(store[i].session_id, session_id) == 0) return (int)i;
    }
    return -1;
}

static void set_token(const char* session_id, const char* token) {
    int idx = find_entry_index(session_id);
    if (idx >= 0) {
        free(store[idx].token);
        store[idx].token = my_strdup(token);
        return;
    }
    if (store_count == store_cap) {
        size_t new_cap = store_cap == 0 ? 8 : store_cap * 2;
        Entry* n = (Entry*)realloc(store, new_cap * sizeof(Entry));
        if (!n) return;
        store = n;
        store_cap = new_cap;
    }
    store[store_count].session_id = my_strdup(session_id);
    store[store_count].token = my_strdup(token);
    store_count++;
}

static char* get_token(const char* session_id) {
    int idx = find_entry_index(session_id);
    if (idx < 0) return NULL;
    return store[idx].token;
}

static void remove_session(const char* session_id) {
    int idx = find_entry_index(session_id);
    if (idx < 0) return;
    free(store[idx].session_id);
    free(store[idx].token);
    if (idx != (int)store_count - 1) {
        store[idx] = store[store_count - 1];
    }
    store_count--;
}

char* issue_token(const char* session_id) {
    unsigned char buf[32];
    if (!get_random_bytes(buf, sizeof(buf))) return NULL;
    char* token = base64url_encode(buf, sizeof(buf));
    if (!token) return NULL;
    set_token(session_id, token);
    // return a duplicate for caller management
    char* ret = my_strdup(token);
    free(token);
    return ret;
}

int validate_token(const char* session_id, const char* token) {
    if (!session_id || !token) return 0;
    char* expected = get_token(session_id);
    if (!expected) return 0;
    return constant_time_equals(expected, token);
}

char* rotate_token(const char* session_id) {
    return issue_token(session_id);
}

void invalidate_session(const char* session_id) {
    remove_session(session_id);
}

int main(void) {
    // 5 test cases
    const char* s1 = "sessionA";
    const char* s2 = "sessionB";

    char* t1 = issue_token(s1);
    printf("Test1 (valid token): %s\n", validate_token(s1, t1) ? "true" : "false");

    printf("Test2 (invalid token): %s\n", validate_token(s1, "invalidtoken") ? "true" : "false");

    printf("Test3 (missing session): %s\n", validate_token("noSession", t1) ? "true" : "false");

    char* t2 = issue_token(s2);
    (void)t2; // unused variable in tests beyond issuance
    printf("Test4 (cross-session misuse): %s\n", validate_token(s2, t1) ? "true" : "false");

    char* old = t1;
    char* rotated = rotate_token(s1);
    int oldValid = validate_token(s1, old);
    int newValid = validate_token(s1, rotated);
    printf("Test5 (rotate old invalid/new valid): %s\n", (!oldValid && newValid) ? "true" : "false");

    free(t1);
    free(rotated);
    free(t2);

    // Cleanup store
    while (store_count > 0) {
        free(store[store_count - 1].session_id);
        free(store[store_count - 1].token);
        store_count--;
    }
    free(store);
    return 0;
}