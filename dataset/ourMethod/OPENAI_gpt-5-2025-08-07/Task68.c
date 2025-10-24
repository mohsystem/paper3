#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* dn;
    const char* uid;
    const char* cn;
    const char* mail;
} Entry;

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_INPUT = 1,
    STATUS_NOT_FOUND = 2
} Status;

typedef struct {
    Status status;
    char* info;   // non-NULL on success
    char* error;  // non-NULL on failure
} Result;

static int is_label_valid(const char* s) {
    size_t len = s ? strlen(s) : 0;
    if (len == 0 || len > 63) return 0;
    if (s[0] == '-' || s[len - 1] == '-') return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-')) return 0;
    }
    return 1;
}

static char* str_to_lower_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)tolower((unsigned char)s[i]);
    }
    out[len] = '\0';
    return out;
}

static int ends_with_case_insensitive(const char* s, const char* suffix) {
    if (!s || !suffix) return 0;
    size_t ls = strlen(s);
    size_t lsf = strlen(suffix);
    if (ls < lsf) return 0;
    char* tail = str_to_lower_dup(s + (ls - lsf));
    char* suf  = str_to_lower_dup(suffix);
    if (!tail || !suf) {
        free(tail);
        free(suf);
        return 0;
    }
    int eq = (strcmp(tail, suf) == 0);
    free(tail);
    free(suf);
    return eq;
}

// Parses input into base DN. Supports "dc=example,dc=com" or "example.com"
static int parse_dc_to_base_dn(const char* input, char* outBaseDn, size_t outSize) {
    if (!input || !outBaseDn || outSize == 0) return 0;
    outBaseDn[0] = '\0';
    size_t inLen = strlen(input);
    if (inLen == 0 || inLen > 253) return 0;

    int startsWithDC = 0;
    if (inLen >= 3 && (input[0] == 'd' || input[0] == 'D') && (input[1] == 'c' || input[1] == 'C') && input[2] == '=') {
        startsWithDC = 1;
    }

    char buffer[512];
    buffer[0] = '\0';

    if (startsWithDC) {
        // Validate and copy parts
        const char* p = input;
        size_t written = 0;
        while (*p) {
            const char* comma = strchr(p, ',');
            size_t partLen = (size_t)((comma ? comma : (input + inLen)) - p);
            if (partLen < 4) return 0; // minimum "dc=x"
            if (!((p[0] == 'd' || p[0] == 'D') && (p[1] == 'c' || p[1] == 'C') && p[2] == '=')) return 0;
            char label[128];
            size_t labLen = partLen - 3;
            if (labLen >= sizeof(label)) return 0;
            memcpy(label, p + 3, labLen);
            label[labLen] = '\0';
            if (!is_label_valid(label)) return 0;

            int n = snprintf(buffer + written, sizeof(buffer) - written, "%sdc=%s", (written ? "," : ""), label);
            if (n < 0) return 0;
            written += (size_t)n;
            if (written >= sizeof(buffer)) return 0;
            if (!comma) break;
            p = comma + 1;
        }
    } else {
        // Domain form
        const char* p = input;
        size_t written = 0;
        while (*p) {
            const char* dot = strchr(p, '.');
            size_t labLen = (size_t)((dot ? dot : (input + inLen)) - p);
            if (labLen == 0 || labLen > 63) return 0;
            char label[64 + 1];
            if (labLen >= sizeof(label)) return 0;
            memcpy(label, p, labLen);
            label[labLen] = '\0';
            if (!is_label_valid(label)) return 0;

            int n = snprintf(buffer + written, sizeof(buffer) - written, "%sdc=%s", (written ? "," : ""), label);
            if (n < 0) return 0;
            written += (size_t)n;
            if (written >= sizeof(buffer)) return 0;
            if (!dot) break;
            p = dot + 1;
        }
    }

    if (strlen(buffer) + 1 > outSize) return 0;
    snprintf(outBaseDn, outSize, "%s", buffer);
    return 1;
}

static int validate_username(const char* username) {
    if (!username) return 0;
    size_t len = strlen(username);
    if (len == 0 || len > 64) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c == '-' || c == '_' || c == '.')) return 0;
    }
    return 1;
}

// RFC 4515 escaping for LDAP filter values
static char* escape_ldap_value(const char* in) {
    if (!in) return NULL;
    size_t len = strlen(in);
    // Worst-case each char becomes 3 chars (e.g., \2A)
    size_t maxOut = len * 3 + 1;
    char* out = (char*)malloc(maxOut);
    if (!out) return NULL;
    size_t w = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (c == '*') {
            if (w + 3 >= maxOut) { free(out); return NULL; }
            out[w++] = '\\'; out[w++] = '2'; out[w++] = 'A';
        } else if (c == '(') {
            if (w + 3 >= maxOut) { free(out); return NULL; }
            out[w++] = '\\'; out[w++] = '2'; out[w++] = '8';
        } else if (c == ')') {
            if (w + 3 >= maxOut) { free(out); return NULL; }
            out[w++] = '\\'; out[w++] = '2'; out[w++] = '9';
        } else if (c == '\\') {
            if (w + 3 >= maxOut) { free(out); return NULL; }
            out[w++] = '\\'; out[w++] = '5'; out[w++] = 'C';
        } else if (c == '\0') {
            if (w + 3 >= maxOut) { free(out); return NULL; }
            out[w++] = '\\'; out[w++] = '0'; out[w++] = '0';
        } else {
            if (w + 1 >= maxOut) { free(out); return NULL; }
            out[w++] = (char)c;
        }
    }
    out[w] = '\0';
    return out;
}

static char* build_filter(const char* username) {
    char* escaped = escape_ldap_value(username);
    if (!escaped) return NULL;
    size_t len = strlen(escaped) + 3; // (uid=...) + ')' and '('
    char* filter = (char*)malloc(len + 5); // "(uid=" is 5 chars including '('
    if (!filter) {
        free(escaped);
        return NULL;
    }
    int n = snprintf(filter, len + 5, "(uid=%s)", escaped);
    free(escaped);
    if (n < 0) {
        free(filter);
        return NULL;
    }
    return filter;
}

static const Entry* get_directory(size_t* count) {
    static const Entry dir[] = {
        {"uid=alice,ou=People,dc=example,dc=com", "alice", "Alice Example", "alice@example.com"},
        {"uid=bob,ou=People,dc=example,dc=com", "bob", "Bob Builder", "bob@example.com"},
        {"uid=john.doe,ou=People,dc=example,dc=com", "john.doe", "John Doe", "john.doe@example.com"},
        {"uid=admin,ou=Admins,dc=example,dc=com", "admin", "Administrator", "admin@example.com"},
        {"uid=carol,ou=Users,dc=test,dc=org", "carol", "Carol Test", "carol@test.org"}
    };
    if (count) *count = sizeof(dir) / sizeof(dir[0]);
    return dir;
}

static int parse_uid_equals_filter(const char* filter, char* out, size_t outSize) {
    if (!filter || !out || outSize == 0) return 0;
    size_t len = strlen(filter);
    if (len < 7) return 0; // "(uid=x)"
    if (filter[0] != '(' || filter[len - 1] != ')') return 0;
    const char* inner = filter + 1;
    size_t innerLen = len - 2;
    if (innerLen < 5) return 0;
    if (strncmp(inner, "uid=", 4) != 0) return 0;
    const char* value = inner + 4;
    size_t vlen = innerLen - 4;
    if (vlen == 0 || vlen + 1 > outSize) return 0;
    memcpy(out, value, vlen);
    out[vlen] = '\0';
    return 1;
}

static char* search_ldap_localhost(const char* baseDn, const char* filter) {
    // Returns malloc'ed user info string on success or NULL if not found/error
    char uid[128];
    if (!parse_uid_equals_filter(filter, uid, sizeof(uid))) {
        return NULL;
    }
    size_t count = 0;
    const Entry* dir = get_directory(&count);
    for (size_t i = 0; i < count; ++i) {
        if (ends_with_case_insensitive(dir[i].dn, baseDn) && strcmp(dir[i].uid, uid) == 0) {
            // Build info string
            const char* fmt = "dn: %s\ncn: %s\nmail: %s\nuid: %s\n";
            int needed = snprintf(NULL, 0, fmt, dir[i].dn, dir[i].cn, dir[i].mail, dir[i].uid);
            if (needed < 0) return NULL;
            char* info = (char*)malloc((size_t)needed + 1);
            if (!info) return NULL;
            snprintf(info, (size_t)needed + 1, fmt, dir[i].dn, dir[i].cn, dir[i].mail, dir[i].uid);
            return info;
        }
    }
    return NULL;
}

static Result process_request(const char* dcInput, const char* usernameInput) {
    Result res;
    res.status = STATUS_INVALID_INPUT;
    res.info = NULL;
    res.error = NULL;

    char baseDn[512];
    if (!parse_dc_to_base_dn(dcInput, baseDn, sizeof(baseDn))) {
        res.error = (char*)malloc(32);
        if (res.error) snprintf(res.error, 32, "Invalid domain component");
        return res;
    }
    if (!validate_username(usernameInput)) {
        res.error = (char*)malloc(23);
        if (res.error) snprintf(res.error, 23, "Invalid username format");
        return res;
    }
    char* filter = build_filter(usernameInput);
    if (!filter) {
        res.error = (char*)malloc(26);
        if (res.error) snprintf(res.error, 26, "Failed to build LDAP filter");
        return res;
    }

    char* info = search_ldap_localhost(baseDn, filter);
    free(filter);
    if (info) {
        res.status = STATUS_OK;
        res.info = info;
        return res;
    } else {
        res.status = STATUS_NOT_FOUND;
        res.error = (char*)malloc(16);
        if (res.error) snprintf(res.error, 16, "User not found");
        return res;
    }
}

static void free_result(Result* r) {
    if (!r) return;
    if (r->info) { free(r->info); r->info = NULL; }
    if (r->error) { free(r->error); r->error = NULL; }
}

int main(int argc, char* argv[]) {
    struct {
        const char* dc;
        const char* user;
    } tests[5] = {
        {"example.com", "alice"},            // Found
        {"dc=example,dc=com", "john.doe"},   // Found
        {"test.org", "carol"},               // Found
        {"example.com", "doesnotexist"},     // Not found
        {"example.com", "alice*)(uid=*"}     // Invalid username (rejected)
    };

    int hasArgs = (argc >= 3);
    if (hasArgs) {
        tests[0].dc = argv[1];
        tests[0].user = argv[2];
    }

    int total = hasArgs ? 1 : 5;
    for (int i = 0; i < total; ++i) {
        Result r = process_request(tests[i].dc, tests[i].user);
        printf("Test %d dc=\"%s\" user=\"%s\"\n", i + 1, tests[i].dc, tests[i].user);
        if (r.status == STATUS_OK) {
            printf("OK\n%s", r.info ? r.info : "");
        } else {
            printf("ERROR: %s\n", r.error ? r.error : "Unknown error");
        }
        if (i + 1 < total) printf("-----\n");
        free_result(&r);
    }
    return 0;
}