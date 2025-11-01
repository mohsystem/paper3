#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_EMAIL_LEN 254

static char USER_DB[MAX_USERS][MAX_EMAIL_LEN + 1];
static int USER_DB_COUNT = 0;

static char MAILING_LIST[MAX_USERS][MAX_EMAIL_LEN + 1];
static int MAILING_LIST_COUNT = 0;

static int is_allowed_char(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '.' || c == '_' || c == '%' || c == '+' || c == '-' || c == '@';
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z');
}

static int is_alnum2(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

static void to_lower_inplace(char* s) {
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static void trim_inplace(char* s) {
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static int contains_double_dot(const char* s) {
    for (; *s && *(s + 1); ++s) {
        if (*s == '.' && *(s + 1) == '.') return 1;
    }
    return 0;
}

static int sanitize_email(const char* input, char* out, size_t out_size) {
    if (input == NULL || out == NULL || out_size < 1) return 0;

    // Copy and trim
    size_t in_len = strnlen(input, 4096);
    if (in_len >= 4096) return 0; // overly long input
    char buf[4096];
    memcpy(buf, input, in_len);
    buf[in_len] = '\0';

    trim_inplace(buf);
    to_lower_inplace(buf);
    size_t len = strlen(buf);
    if (len < 3 || len > MAX_EMAIL_LEN) return 0;

    // Allowed chars
    for (size_t i = 0; i < len; i++) {
        if (!is_allowed_char(buf[i])) return 0;
    }

    // '@' checks
    char* at = strchr(buf, '@');
    if (at == NULL) return 0;
    if (at == buf || *(at + 1) == '\0') return 0;
    if (strchr(at + 1, '@') != NULL) return 0;

    if (contains_double_dot(buf)) return 0;

    // local and domain parts
    size_t local_len = (size_t)(at - buf);
    char* domain = at + 1;
    size_t domain_len = strlen(domain);
    if (local_len == 0 || domain_len == 0) return 0;

    char* last_dot = strrchr(domain, '.');
    if (last_dot == NULL) return 0;
    size_t pos_last_dot = (size_t)(last_dot - domain);
    if (pos_last_dot == 0 || pos_last_dot >= domain_len - 2) return 0;

    // TLD checks
    const char* tld = last_dot + 1;
    size_t tld_len = strlen(tld);
    if (tld_len < 2 || tld_len > 63) return 0;
    for (size_t i = 0; i < tld_len; i++) {
        if (!is_alpha(tld[i])) return 0;
    }

    // Validate domain labels
    char domain_copy[MAX_EMAIL_LEN + 1];
    if (domain_len > MAX_EMAIL_LEN) return 0;
    memcpy(domain_copy, domain, domain_len + 1);

    char* saveptr = NULL;
    char* label = strtok_r(domain_copy, ".", &saveptr);
    while (label != NULL) {
        size_t l = strlen(label);
        if (l == 0) return 0;
        if (label[0] == '-' || label[l - 1] == '-') return 0;
        for (size_t i = 0; i < l; i++) {
            if (!(is_alnum2(label[i]) || label[i] == '-')) return 0;
        }
        label = strtok_r(NULL, ".", &saveptr);
    }

    // copy out
    if (len >= out_size) return 0;
    memcpy(out, buf, len + 1);
    return 1;
}

static int db_find(char db[][MAX_EMAIL_LEN + 1], int count, const char* email) {
    for (int i = 0; i < count; i++) {
        if (strcmp(db[i], email) == 0) return i;
    }
    return -1;
}

static void db_delete(char db[][MAX_EMAIL_LEN + 1], int* count, const char* email) {
    int idx = db_find(db, *count, email);
    if (idx >= 0) {
        for (int i = idx; i < *count - 1; i++) {
            memcpy(db[i], db[i + 1], MAX_EMAIL_LEN + 1);
        }
        (*count)--;
    }
}

static char* make_msg(const char* msg) {
    size_t len = strlen(msg) + 1;
    char* p = (char*)malloc(len);
    if (!p) return NULL;
    memcpy(p, msg, len);
    return p;
}

char* unsubscribe_get(const char* email_param) {
    char email[MAX_EMAIL_LEN + 1];
    if (!sanitize_email(email_param, email, sizeof(email))) {
        return make_msg("Invalid email parameter.");
    }
    int idx = db_find(USER_DB, USER_DB_COUNT, email);
    if (idx >= 0) {
        db_delete(USER_DB, &USER_DB_COUNT, email);
        db_delete(MAILING_LIST, &MAILING_LIST_COUNT, email);
        return make_msg("You have been unsubscribed.");
    } else {
        return make_msg("The provided email is not subscribed.");
    }
}

static void seed(void) {
    USER_DB_COUNT = 0;
    MAILING_LIST_COUNT = 0;
    const char* seeds[] = {
        "alice@example.com",
        "bob@example.org",
        "carol.test+news@sub.domain.com"
    };
    for (size_t i = 0; i < sizeof(seeds)/sizeof(seeds[0]); i++) {
        char sanitized[MAX_EMAIL_LEN + 1];
        if (sanitize_email(seeds[i], sanitized, sizeof(sanitized))) {
            if (USER_DB_COUNT < MAX_USERS) {
                strncpy(USER_DB[USER_DB_COUNT], sanitized, MAX_EMAIL_LEN);
                USER_DB[USER_DB_COUNT][MAX_EMAIL_LEN] = '\0';
                USER_DB_COUNT++;
            }
            if (MAILING_LIST_COUNT < MAX_USERS) {
                strncpy(MAILING_LIST[MAILING_LIST_COUNT], sanitized, MAX_EMAIL_LEN);
                MAILING_LIST[MAILING_LIST_COUNT][MAX_EMAIL_LEN] = '\0';
                MAILING_LIST_COUNT++;
            }
        }
    }
}

int main(void) {
    seed();
    const char* tests[5] = {
        "alice@example.com",
        " ALICE@EXAMPLE.COM ",
        "carol.test+news@sub.domain.com",
        "dave@example.com",
        "bad@@example..com"
    };
    for (int i = 0; i < 5; i++) {
        char* result = unsubscribe_get(tests[i]);
        if (result) {
            printf("Input: %s -> %s\n", tests[i], result);
            free(result);
        } else {
            printf("Input: %s -> [error allocating message]\n", tests[i]);
        }
    }
    return 0;
}