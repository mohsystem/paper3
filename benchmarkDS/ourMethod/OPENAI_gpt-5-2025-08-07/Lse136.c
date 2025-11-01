#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_URL_LEN 2048
#define MAX_PATH_LEN 256
#define MAX_PARAM_LEN 256
#define MAX_PAIRS 10

typedef struct {
    unsigned char salt[16];
    unsigned char hash[32];
    int iterations;
    char username[65];
} UserRecord;

static void secure_zero(void* v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(v, n, 0, n);
#else
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) { *p++ = 0; }
#endif
}

static int is_username_valid(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int is_password_valid_policy(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    if (len < 12 || len > 128) return 0;
    int U=0,L=0,D=0,S=0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) U = 1;
        else if (islower(c)) L = 1;
        else if (isdigit(c)) D = 1;
        else S = 1;
    }
    return U && L && D && S;
}

static int pbkdf2_sha256(const char* password, const unsigned char* salt, int salt_len, int iterations, unsigned char* out, int out_len) {
    if (!password || !salt || !out) return 0;
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len, iterations, EVP_sha256(), out_len, out) != 1) {
        return 0;
    }
    return 1;
}

static int create_user_record(const char* username, const char* password, int iterations, UserRecord* rec) {
    if (!username || !password || !rec) return 0;
    memset(rec, 0, sizeof(*rec));
    size_t ulen = strlen(username);
    if (ulen > 64) return 0;
    memcpy(rec->username, username, ulen);
    rec->username[ulen] = '\0';
    if (RAND_bytes(rec->salt, (int)sizeof(rec->salt)) != 1) return 0;
    rec->iterations = iterations;
    if (!pbkdf2_sha256(password, rec->salt, (int)sizeof(rec->salt), rec->iterations, rec->hash, (int)sizeof(rec->hash))) return 0;
    return 1;
}

static int verify_password(const char* password, const UserRecord* rec) {
    if (!password || !rec) return 0;
    unsigned char derived[32];
    if (!pbkdf2_sha256(password, rec->salt, (int)sizeof(rec->salt), rec->iterations, derived, (int)sizeof(derived))) {
        return 0;
    }
    int ok = (CRYPTO_memcmp(derived, rec->hash, sizeof(derived)) == 0);
    secure_zero(derived, sizeof(derived));
    return ok;
}

typedef struct {
    char key[MAX_PARAM_LEN + 1];
    char val[MAX_PARAM_LEN + 1];
} Pair;

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static int url_percent_decode(const char* in, char* out, size_t out_cap) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        if (oi >= out_cap) return 0;
        char c = in[i];
        if (c == '+') {
            out[oi++] = ' ';
        } else if (c == '%' && in[i+1] && in[i+2]) {
            int hi = hexval(in[i+1]);
            int lo = hexval(in[i+2]);
            if (hi < 0 || lo < 0) return 0;
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            out[oi++] = c;
        }
    }
    if (oi > out_cap) return 0;
    out[oi] = '\0';
    return 1;
}

static void parse_query(const char* query, Pair* pairs, size_t* pair_count) {
    *pair_count = 0;
    if (!query || !*query) return;
    char buf[1025];
    size_t qlen = strlen(query);
    if (qlen > 1024) return;
    memcpy(buf, query, qlen);
    buf[qlen] = '\0';

    char* saveptr = NULL;
    char* token = strtok_r(buf, "&", &saveptr);
    while (token && *pair_count < MAX_PAIRS) {
        char* eq = strchr(token, '=');
        char key[ MAX_PARAM_LEN + 1 ];
        char val[ MAX_PARAM_LEN + 1 ];
        if (eq) {
            *eq = '\0';
            if (!url_percent_decode(token, key, MAX_PARAM_LEN)) { *pair_count = 0; return; }
            if (!url_percent_decode(eq + 1, val, MAX_PARAM_LEN)) { *pair_count = 0; return; }
        } else {
            if (!url_percent_decode(token, key, MAX_PARAM_LEN)) { *pair_count = 0; return; }
            val[0] = '\0';
        }
        // Insert only first occurrence
        int exists = 0;
        for (size_t i = 0; i < *pair_count; i++) {
            if (strcmp(pairs[i].key, key) == 0) { exists = 1; break; }
        }
        if (!exists) {
            strncpy(pairs[*pair_count].key, key, MAX_PARAM_LEN);
            pairs[*pair_count].key[MAX_PARAM_LEN] = '\0';
            strncpy(pairs[*pair_count].val, val, MAX_PARAM_LEN);
            pairs[*pair_count].val[MAX_PARAM_LEN] = '\0';
            (*pair_count)++;
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
}

static int extract_path(const char* url, char* out_path, size_t out_cap) {
    if (!url || !out_path || out_cap == 0) return 0;
    size_t ulen = strlen(url);
    if (ulen > MAX_URL_LEN) return 0;
    size_t start = 0;
    const char* scheme = strstr(url, "://");
    if (scheme) {
        const char* slash = strchr(scheme + 3, '/');
        start = slash ? (size_t)(slash - url) : ulen;
    } else {
        start = 0;
    }
    size_t end = ulen;
    const char* q = strchr(url + start, '?');
    const char* h = strchr(url + start, '#');
    if (q && h) end = (size_t)((q < h ? q : h) - url);
    else if (q) end = (size_t)(q - url);
    else if (h) end = (size_t)(h - url);

    size_t plen = (end > start) ? (end - start) : 0;
    if (plen == 0) {
        if (out_cap < 2) return 0;
        out_path[0] = '/';
        out_path[1] = '\0';
        return 1;
    }
    if (plen >= out_cap || plen > MAX_PATH_LEN) return 0;
    memcpy(out_path, url + start, plen);
    out_path[plen] = '\0';
    return 1;
}

static const char* extract_query(const char* url) {
    const char* q = strchr(url, '?');
    if (!q) return "";
    return q + 1;
}

// The requested function
const char* check_mod(const char* url, const UserRecord* store, size_t store_len) {
    if (!url || !store) return "false";
    char path[MAX_PATH_LEN + 1];
    if (!extract_path(url, path, sizeof(path))) return "false";
    if (strcmp(path, "/check_mod") != 0) return "false";

    Pair pairs[MAX_PAIRS];
    size_t count = 0;
    parse_query(extract_query(url), pairs, &count);
    if (count == 0) return "false";

    const char* username = NULL;
    const char* password = NULL;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(pairs[i].key, "username") == 0) username = pairs[i].val;
        else if (strcmp(pairs[i].key, "password") == 0) password = pairs[i].val;
    }
    if (!username || !password) return "false";
    if (!is_username_valid(username)) return "false";
    if (!is_password_valid_policy(password)) return "false";

    const UserRecord* rec = NULL;
    for (size_t i = 0; i < store_len; i++) {
        if (strcmp(store[i].username, username) == 0) { rec = &store[i]; break; }
    }
    if (!rec) return "false";
    return verify_password(password, rec) ? "true" : "false";
}

int main(void) {
    UserRecord store[1];
    if (!create_user_record("alice", "Str0ng-P@ssw0rd!", 210000, &store[0])) {
        printf("false\nfalse\nfalse\nfalse\nfalse\n");
        return 0;
    }

    const char* t1 = "http://localhost/check_mod?username=alice&password=Str0ng-P%40ssw0rd%21"; // true
    const char* t2 = "/check_mod?username=alice&password=Wrong-P%40ssw0rd%21"; // false
    const char* t3 = "/other?username=alice&password=Str0ng-P%40ssw0rd%21"; // false
    const char* t4 = "/check_mod?username=alice"; // false
    const char* t5 = "/check_mod?username=alice&password=short1!A"; // false

    printf("%s\n", check_mod(t1, store, 1));
    printf("%s\n", check_mod(t2, store, 1));
    printf("%s\n", check_mod(t3, store, 1));
    printf("%s\n", check_mod(t4, store, 1));
    printf("%s\n", check_mod(t5, store, 1));
    return 0;
}