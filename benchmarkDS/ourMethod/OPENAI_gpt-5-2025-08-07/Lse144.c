#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define MAX_EMAIL_LEN 254

static size_t bounded_strlen(const char *s, size_t max) {
    size_t i = 0;
    if (!s) return 0;
    while (i < max && s[i] != '\0') i++;
    return i;
}

static bool is_valid_email(const char *email) {
    if (email == NULL) return false;

    // Trim leading/trailing spaces
    const char *start = email;
    while (*start && isspace((unsigned char)*start)) start++;
    const char *end = email + bounded_strlen(email, MAX_EMAIL_LEN + 10);
    if (end == email) return false;
    end--; // last char
    while (end >= start && isspace((unsigned char)*end)) end--;
    if (end < start) return false;

    size_t len = (size_t)(end - start + 1);
    if (len < 3 || len > MAX_EMAIL_LEN) return false;

    // find '@'
    const char *at = NULL;
    for (const char *p = start; p <= end; ++p) {
        if (*p == '@') {
            if (at != NULL) return false; // multiple @
            at = p;
        }
    }
    if (at == NULL || at == start || at == end) return false;

    // local part
    const char *lstart = start;
    const char *lend = at - 1;
    size_t l_len = (size_t)(lend - lstart + 1);
    if (l_len == 0 || l_len > 64) return false;
    if (*lstart == '.' || *lend == '.') return false;
    bool prev_dot = false;
    for (const char *p = lstart; p <= lend; ++p) {
        unsigned char c = (unsigned char)*p;
        bool ok = isalnum(c) || c == '.' || c == '_' || c == '+' || c == '-';
        if (!ok) return false;
        if (c == '.') {
            if (prev_dot) return false;
            prev_dot = true;
        } else {
            prev_dot = false;
        }
    }

    // domain part
    const char *dstart = at + 1;
    const char *dend = end;
    // must contain at least one dot
    bool has_dot = false;
    for (const char *p = dstart; p <= dend; ++p) {
        if (*p == '.') { has_dot = true; break; }
    }
    if (!has_dot) return false;

    // split labels by dot and validate
    size_t label_len = 0;
    bool in_label = false;
    const char *p = dstart;
    const char *label_start = dstart;
    size_t label_count = 0;
    for (; p <= dend + 1; ++p) {
        if (p <= dend && *p != '.') {
            if (!in_label) {
                in_label = true;
                label_start = p;
                label_len = 0;
            }
            unsigned char c = (unsigned char)*p;
            if (!(isalnum(c) || c == '-')) return false;
            label_len++;
            if (label_len > 63) return false;
        } else {
            if (!in_label) return false; // empty label
            // label ends at p-1
            if (*label_start == '-' || *(p - 1) == '-') return false;
            label_count++;
            in_label = false;
        }
    }
    if (label_count < 2) return false;

    // Check TLD length >= 2
    // Find last dot
    const char *last_dot = dend;
    while (last_dot >= dstart && *last_dot != '.') last_dot--;
    if (last_dot < dstart || last_dot == dend) return false;
    size_t tld_len = (size_t)(dend - last_dot);
    if (tld_len < 2) return false;

    return true;
}

static int normalize_email(const char *email, char *out, size_t outsz) {
    if (!email || !out || outsz == 0) return -1;
    const char *start = email;
    while (*start && isspace((unsigned char)*start)) start++;
    const char *end = email + bounded_strlen(email, MAX_EMAIL_LEN + 10);
    if (end == email) return -1;
    end--;
    while (end >= start && isspace((unsigned char)*end)) end--;
    if (end < start) return -1;
    size_t len = (size_t)(end - start + 1);
    if (len >= outsz) return -1;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)tolower((unsigned char)start[i]);
    }
    out[len] = '\0';
    return 0;
}

static bool db_contains(const char **db, size_t db_sz, const char *email_norm) {
    for (size_t i = 0; i < db_sz; ++i) {
        if (!db[i]) continue;
        // db is expected to be normalized already (lowercase, trimmed)
        if (strncmp(db[i], email_norm, MAX_EMAIL_LEN) == 0 && db[i][strlen(email_norm)] == '\0') {
            return true;
        }
    }
    return false;
}

// Example script: returns 0 for @example.com, else non-zero
static int example_script(const char *email) {
    const char *suffix = "@example.com";
    size_t elen = bounded_strlen(email, MAX_EMAIL_LEN + 1);
    size_t slen = strlen(suffix);
    if (elen >= slen) {
        if (strncmp(email + (elen - slen), suffix, slen) == 0) {
            return 0;
        }
    }
    return 7;
}

// Returns 1 if email exists and unsubscribe script returns 0, else 0
static int process_unsubscribe(const char *email,
                               const char **database, size_t db_sz,
                               int (*script)(const char *)) {
    if (!email || !database || db_sz == 0 || !script) return 0;
    if (!is_valid_email(email)) return 0;

    char norm[MAX_EMAIL_LEN + 1];
    if (normalize_email(email, norm, sizeof(norm)) != 0) return 0;

    if (!db_contains(database, db_sz, norm)) return 0;

    int rc = script(norm);
    return (rc == 0) ? 1 : 0;
}

int main(void) {
    // Database: normalized lowercase emails
    const char *db[] = {
        "alice@example.com",
        "bob@example.com",
        "carol@block.example",
        "dave@example.org",
        "eve@example.net"
    };
    const size_t db_sz = sizeof(db) / sizeof(db[0]);

    const char *tests[5] = {
        "alice@example.com",     // exists, script=0 -> 1
        "carol@block.example",   // exists, script!=0 -> 0
        "frank@example.com",     // not exists -> 0
        "bad@@example.com",      // invalid -> 0
        "dave@example.org"       // exists, script!=0 -> 0
    };

    for (int i = 0; i < 5; ++i) {
        int res = process_unsubscribe(tests[i], db, db_sz, example_script);
        printf("Test %d: %d\n", i + 1, res);
    }
    return 0;
}