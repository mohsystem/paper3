#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

static bool is_allowed_local_char(char c) {
    return (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-');
}

static bool is_allowed_domain_char(char c) {
    return (isalnum((unsigned char)c) || c == '.' || c == '-');
}

static bool is_valid_email(const char* email) {
    if (email == NULL) return false;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return false;

    const char* at = strchr(email, '@');
    if (!at) return false;
    // Ensure only one '@'
    if (strchr(at + 1, '@')) return false;

    size_t local_len = (size_t)(at - email);
    size_t domain_len = len - local_len - 1;
    if (local_len == 0 || local_len > 64) return false;
    if (domain_len < 3) return false; // minimal a.b

    // Validate local part
    if (email[0] == '.' || email[local_len - 1] == '.') return false;
    for (size_t i = 0; i < local_len; ++i) {
        if (!is_allowed_local_char(email[i])) return false;
        if (email[i] == '.' && i + 1 < local_len && email[i + 1] == '.') return false;
    }

    const char* domain = at + 1;
    if (domain[0] == '-' || domain[domain_len - 1] == '-' ||
        domain[0] == '.' || domain[domain_len - 1] == '.') return false;

    bool has_dot = false;
    for (size_t i = 0; i < domain_len; ++i) {
        char c = domain[i];
        if (!is_allowed_domain_char(c)) return false;
        if (c == '.') {
            has_dot = true;
            if (i + 1 < domain_len && domain[i + 1] == '.') return false;
        }
    }
    if (!has_dot) return false;

    // TLD at least 2 chars
    const char* last_dot = strrchr(domain, '.');
    if (!last_dot || strlen(last_dot + 1) < 2) return false;

    return true;
}

static bool db_contains(const char* email, const char* db[], size_t db_size) {
    if (!email || !db) return false;
    for (size_t i = 0; i < db_size; ++i) {
        if (db[i] && strcmp(db[i], email) == 0) {
            return true;
        }
    }
    return false;
}

static int run_unsubscribe_script(const char* email) {
    // Secure stub simulating external script return code.
    // In real code, prefer execve/posix_spawn with fixed path and sanitized argv, avoiding shell.
    if (!email) return 1;
    // Convert to lowercase safely
    char buf[256];
    size_t n = strlen(email);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (size_t i = 0; i < n; ++i) {
        buf[i] = (char)tolower((unsigned char)email[i]);
    }
    buf[n] = '\0';

    if (strncmp(buf, "error", 5) == 0 || strstr(buf, "fail") != NULL) {
        return 2; // simulate failure
    }
    return 0; // success
}

int process_unsubscribe(const char* email, const char* database[], size_t db_size) {
    if (!is_valid_email(email)) return 0;
    if (!db_contains(email, database, db_size)) return 0;
    int code = run_unsubscribe_script(email);
    return (code == 0) ? 1 : 0;
}

int main(void) {
    const char* db[] = {
        "alice@example.com",
        "bob@sample.org",
        "charlie@test.net",
        "dora@domain.com",
        "error@test.net"
    };
    size_t db_size = sizeof(db) / sizeof(db[0]);

    const char* tests[5] = {
        "alice@example.com",   // exists, script OK -> 1
        "unknown@x.com",       // not exists -> 0
        "error@test.net",      // exists, script fails -> 0
        "invalid",             // invalid -> 0
        "dora@domain.com"      // exists, script OK -> 1
    };

    for (int i = 0; i < 5; ++i) {
        int res = process_unsubscribe(tests[i], db, db_size);
        printf("Email: %s => %d\n", tests[i], res);
    }

    return 0;
}