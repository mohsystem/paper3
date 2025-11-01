#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_EMAIL_LEN 254
#define MAX_LOCAL_LEN 64
#define MAX_RECORDS 64
#define STR_BUF 256

static void to_lower_trim_copy(const char* src, char* dst, size_t dstsz, int* ok) {
    *ok = 0;
    if (!src || !dst || dstsz == 0) return;
    // Trim
    size_t len = strlen(src);
    size_t start = 0, end = len;
    while (start < end && isspace((unsigned char)src[start])) start++;
    while (end > start && isspace((unsigned char)src[end - 1])) end--;
    size_t need = end - start;
    if (need >= dstsz) return; // not enough room
    for (size_t i = 0; i < need; ++i) {
        dst[i] = (char)tolower((unsigned char)src[start + i]);
    }
    dst[need] = '\0';
    *ok = 1;
}

static int is_allowed_local(char c) {
    return isalnum((unsigned char)c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}
static int is_allowed_domain(char c) {
    return isalnum((unsigned char)c) || c == '.' || c == '-';
}

static int is_valid_email(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > MAX_EMAIL_LEN) return 0;
    const char* at = strchr(email, '@');
    if (!at) return 0;
    if (strchr(at + 1, '@')) return 0; // multiple '@'
    size_t local_len = (size_t)(at - email);
    if (local_len == 0 || local_len > MAX_LOCAL_LEN) return 0;
    const char* domain = at + 1;
    if (*domain == '\0') return 0;
    for (size_t i = 0; i < local_len; ++i) {
        if (!is_allowed_local(email[i])) return 0;
    }
    for (const char* p = domain; *p; ++p) {
        if (!is_allowed_domain(*p)) return 0;
    }
    const char* last_dot = strrchr(domain, '.');
    if (!last_dot || last_dot == domain || *(last_dot + 1) == '\0') return 0;
    size_t tld_len = strlen(last_dot + 1);
    if (tld_len < 2) return 0;
    for (const char* p = last_dot + 1; *p; ++p) {
        if (!isalpha((unsigned char)*p)) return 0;
    }
    return 1;
}

static int ci_equal(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static int find_index(char arr[][STR_BUF], int count, const char* email_norm) {
    for (int i = 0; i < count; ++i) {
        if (ci_equal(arr[i], email_norm)) return i;
    }
    return -1;
}

static void remove_at(char arr[][STR_BUF], int* count, int idx) {
    if (idx < 0 || idx >= *count) return;
    for (int i = idx; i + 1 < *count; ++i) {
        strncpy(arr[i], arr[i + 1], STR_BUF - 1);
        arr[i][STR_BUF - 1] = '\0';
    }
    (*count)--;
}

// Unsubscribe function: accepts input as parameters and returns output
const char* unsubscribe(const char* email,
                        char database[][STR_BUF], int* db_count,
                        char mailing[][STR_BUF], int* mailing_count) {
    char normalized[STR_BUF];
    int ok = 0;
    to_lower_trim_copy(email, normalized, sizeof(normalized), &ok);
    if (!ok || !is_valid_email(normalized)) {
        return "Invalid email address.";
    }
    int idx = find_index(database, *db_count, normalized);
    if (idx >= 0) {
        remove_at(database, db_count, idx);
        int midx = find_index(mailing, *mailing_count, normalized);
        if (midx >= 0) {
            remove_at(mailing, mailing_count, midx);
        }
        return "Successfully unsubscribed.";
    } else {
        return "Email not subscribed.";
    }
}

int main(void) {
    char database[MAX_RECORDS][STR_BUF];
    int db_count = 0;
    char mailing[MAX_RECORDS][STR_BUF];
    int ml_count = 0;

    const char* db_init[] = {
        "alice@example.com",
        "bob@example.org",
        "upper@example.com",
        "carol+news@sub.example.co",
        "dave@example.net"
    };
    const char* ml_init[] = {
        "alice@example.com",
        "bob@example.org",
        "upper@example.com",
        "carol+news@sub.example.co"
    };

    for (size_t i = 0; i < sizeof(db_init)/sizeof(db_init[0]); ++i) {
        strncpy(database[db_count], db_init[i], STR_BUF - 1);
        database[db_count][STR_BUF - 1] = '\0';
        db_count++;
    }
    for (size_t i = 0; i < sizeof(ml_init)/sizeof(ml_init[0]); ++i) {
        strncpy(mailing[ml_count], ml_init[i], STR_BUF - 1);
        mailing[ml_count][STR_BUF - 1] = '\0';
        ml_count++;
    }

    // 5 test cases
    printf("%s\n", unsubscribe("Alice@Example.com", database, &db_count, mailing, &ml_count));
    printf("%s\n", unsubscribe("unknown@example.com", database, &db_count, mailing, &ml_count));
    printf("%s\n", unsubscribe("invalid-email", database, &db_count, mailing, &ml_count));
    printf("%s\n", unsubscribe("BOB@EXAMPLE.ORG", database, &db_count, mailing, &ml_count));
    printf("%s\n", unsubscribe("dave@example.net", database, &db_count, mailing, &ml_count));

    return 0;
}