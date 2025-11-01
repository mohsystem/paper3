#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EMAIL_LEN 254

typedef struct {
    char **emails;
    size_t size;
    size_t capacity;
    int initialized;
} Database;

static Database g_db = {NULL, 0, 0, 0};

static int ensure_capacity(Database *db, size_t needed) {
    if (needed <= db->capacity) return 1;
    size_t newcap = db->capacity == 0 ? 8 : db->capacity * 2;
    while (newcap < needed) {
        if (newcap > (SIZE_MAX / 2)) return 0;
        newcap *= 2;
    }
    char **newarr = (char **)realloc(db->emails, newcap * sizeof(char *));
    if (!newarr) return 0;
    db->emails = newarr;
    db->capacity = newcap;
    return 1;
}

static int db_add(Database *db, const char *emailLower) {
    if (!ensure_capacity(db, db->size + 1)) return 0;
    size_t len = strnlen(emailLower, MAX_EMAIL_LEN + 1);
    if (len == 0 || len > MAX_EMAIL_LEN) return 0;
    char *cpy = (char *)malloc(len + 1);
    if (!cpy) return 0;
    memcpy(cpy, emailLower, len);
    cpy[len] = '\0';
    db->emails[db->size++] = cpy;
    return 1;
}

static void db_init(Database *db) {
    if (db->initialized) return;
    db->initialized = 1;
    db->emails = NULL;
    db->size = 0;
    db->capacity = 0;
    // preload with lowercase emails
    db_add(db, "user@example.com");
    db_add(db, "alice@test.com");
    db_add(db, "bob@example.org");
    db_add(db, "newsletter@company.com");
    db_add(db, "sales@shop.co");
}

static Database* db_connect(void) {
    db_init(&g_db);
    return &g_db;
}

static void db_free(Database *db) {
    if (!db) return;
    for (size_t i = 0; i < db->size; i++) {
        free(db->emails[i]);
        db->emails[i] = NULL;
    }
    free(db->emails);
    db->emails = NULL;
    db->size = 0;
    db->capacity = 0;
    db->initialized = 0;
}

static int db_find(Database *db, const char *emailLower) {
    for (size_t i = 0; i < db->size; i++) {
        if (strcmp(db->emails[i], emailLower) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int db_remove_at(Database *db, int idx) {
    if (idx < 0 || (size_t)idx >= db->size) return 0;
    free(db->emails[idx]);
    if (db->size > 1 && (size_t)idx != db->size - 1) {
        db->emails[idx] = db->emails[db->size - 1];
    }
    db->emails[db->size - 1] = NULL;
    db->size--;
    return 1;
}

static int is_allowed_local_char(int c) {
    return isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}

static int is_allowed_domain_char(int c) {
    return isalnum(c) || c == '.' || c == '-';
}

// Validate and sanitize email: trim, ASCII-only, length check, simple rules; outputs lowercase
static int sanitize_email(const char *input, char *out, size_t out_size) {
    if (!input || !out || out_size == 0) return 0;

    // Copy with trim and length enforcement
    char buf[MAX_EMAIL_LEN + 1];
    size_t i = 0;
    // Skip leading whitespace
    while (*input && isspace((unsigned char)*input)) input++;
    // Copy up to MAX_EMAIL_LEN or until end
    while (*input && i < MAX_EMAIL_LEN) {
        unsigned char ch = (unsigned char)*input;
        if (ch > 0x7F) return 0; // non-ASCII
        buf[i++] = (char)ch;
        input++;
    }
    if (*input != '\0') return 0; // too long
    // Trim trailing whitespace
    while (i > 0 && isspace((unsigned char)buf[i - 1])) i--;
    buf[i] = '\0';
    if (i == 0) return 0;

    // Must contain exactly one '@'
    size_t at_count = 0;
    size_t at_pos = 0;
    for (size_t k = 0; k < i; k++) {
        if (buf[k] == '@') {
            at_count++;
            at_pos = k;
        }
    }
    if (at_count != 1) return 0;
    if (at_pos == 0 || at_pos == i - 1) return 0; // local and domain cannot be empty

    // Validate local part
    size_t local_len = at_pos;
    if (local_len > 64) return 0;
    if (buf[0] == '.' || buf[local_len - 1] == '.') return 0;
    for (size_t k = 0; k < local_len; k++) {
        int c = (unsigned char)buf[k];
        if (!is_allowed_local_char(c)) return 0;
        if (k > 0 && buf[k] == '.' && buf[k - 1] == '.') return 0; // no consecutive dots
    }

    // Validate domain part
    size_t dom_start = at_pos + 1;
    size_t dom_len = i - dom_start;
    if (dom_len < 3) return 0; // simple minimum
    if (buf[dom_start] == '.' || buf[dom_start + dom_len - 1] == '.') return 0;
    int dot_found = 0;
    int label_len = 0;
    int label_started = 0;
    for (size_t k = dom_start; k < i; k++) {
        int c = (unsigned char)buf[k];
        if (!is_allowed_domain_char(c)) return 0;
        if (buf[k] == '.') {
            dot_found = 1;
            if (k > dom_start && buf[k - 1] == '.') return 0; // no consecutive dots
            if (label_len == 0) return 0; // empty label
            if (!isalnum((unsigned char)buf[k - 1])) return 0; // label must end alnum
            label_len = 0;
            label_started = 0;
        } else {
            if (!label_started) {
                // label must start alnum
                if (!isalnum((unsigned char)buf[k])) return 0;
                label_started = 1;
            }
            label_len++;
        }
    }
    if (!dot_found) return 0;
    if (label_len == 0) return 0;
    if (!isalnum((unsigned char)buf[i - 1])) return 0;

    // Lowercase into out
    size_t out_len = i < (out_size - 1) ? i : (out_size - 1);
    for (size_t k = 0; k < out_len; k++) {
        out[k] = (char)tolower((unsigned char)buf[k]);
    }
    out[out_len] = '\0';
    return 1;
}

const char* unsubscribe(const char *email) {
    Database *db = db_connect();
    char normalized[MAX_EMAIL_LEN + 1];
    if (!sanitize_email(email, normalized, sizeof normalized)) {
        return "Invalid email address.";
    }
    int idx = db_find(db, normalized);
    if (idx >= 0) {
        if (db_remove_at(db, idx)) {
            return "Unsubscribed successfully.";
        } else {
            return "An error occurred.";
        }
    } else {
        return "Email address not found in database.";
    }
}

int main(void) {
    const char *tests[5] = {
        "user@example.com",     // should unsubscribe
        "ALICE@test.com",       // unsubscribe (case-insensitive)
        "nobody@nope.com",      // not found
        "bad@@domain..com",     // invalid
        "user@example.com"      // already unsubscribed -> not found
    };
    for (size_t i = 0; i < 5; i++) {
        const char *res = unsubscribe(tests[i]);
        printf("Input: %s -> %s\n", tests[i], res);
    }
    // Cleanup
    db_free(&g_db);
    return 0;
}