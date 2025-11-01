#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_EMAIL_LEN 254
#define MAX_LOCAL_LEN 64
#define DB_CAPACITY 32

static char* DB[DB_CAPACITY];
static size_t DB_SIZE = 0;

static void secure_free(char** p) {
    if (p && *p) {
        free(*p);
        *p = NULL;
    }
}

static void seed_db();
static int add_to_db(const char* normalized);
static int find_index(const char* normalized);
static int remove_at(size_t idx);

static void rtrim_inplace(char* s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static char* ltrim(const char* s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return (char*)s;
}

static int normalize_email(const char* in, char* out, size_t outsz) {
    if (!in || !out || outsz == 0) return 0;
    // Copy input to a temp buffer to trim
    size_t inlen = strnlen(in, MAX_EMAIL_LEN + 1000);
    if (inlen == 0 || inlen > 10240) return 0; // guard absurdly long input
    char* tmp = (char*)malloc(inlen + 1);
    if (!tmp) return 0;
    memcpy(tmp, in, inlen);
    tmp[inlen] = '\0';

    rtrim_inplace(tmp);
    char* start = ltrim(tmp);

    size_t len = strlen(start);
    if (len == 0 || len > MAX_EMAIL_LEN) {
        free(tmp);
        return 0;
    }
    if (len + 1 > outsz) {
        free(tmp);
        return 0;
    }
    // To lowercase ASCII
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)start[i];
        // Reject control characters
        if (c < 32 || c == 127) {
            free(tmp);
            return 0;
        }
        out[i] = (char)tolower(c);
    }
    out[len] = '\0';
    free(tmp);
    return 1;
}

static int is_label_valid(const char* s, size_t begin, size_t end) {
    if (end <= begin) return 0;
    if (s[begin] == '-' || s[end - 1] == '-') return 0;
    for (size_t i = begin; i < end; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-')) return 0;
    }
    return 1;
}

static int is_valid_email(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > MAX_EMAIL_LEN) return 0;

    // Single '@'
    const char* atp = strchr(email, '@');
    if (!atp) return 0;
    if (strchr(atp + 1, '@')) return 0;

    size_t local_len = (size_t)(atp - email);
    if (local_len == 0 || local_len > MAX_LOCAL_LEN) return 0;

    // Validate local part characters
    for (size_t i = 0; i < local_len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-')) {
            return 0;
        }
    }
    // No consecutive dots in local part
    for (size_t i = 1; i < local_len; i++) {
        if (email[i] == '.' && email[i - 1] == '.') return 0;
    }
    if (email[0] == '.' || email[local_len - 1] == '.') return 0;

    // Domain part
    const char* domain = atp + 1;
    size_t dlen = strlen(domain);
    if (dlen == 0 || dlen > 253) return 0;
    if (domain[0] == '-' || domain[dlen - 1] == '-') return 0;
    if (strstr(domain, "..")) return 0;

    // Must contain at least one dot
    const char* dotp = strchr(domain, '.');
    if (!dotp) return 0;

    // Validate domain labels
    size_t start = 0;
    for (size_t i = 0; i <= dlen; i++) {
        if (i == dlen || domain[i] == '.') {
            if (!is_label_valid(domain, start, i)) return 0;
            start = i + 1;
        }
    }
    // TLD length >= 2
    const char* lastdot = strrchr(domain, '.');
    if (!lastdot) return 0;
    size_t tld_len = strlen(lastdot + 1);
    if (tld_len < 2) return 0;

    return 1;
}

const char* unsubscribe(const char* emailInput) {
    static const char* MSG_INVALID = "Invalid email address.";
    static const char* MSG_OK = "You have been unsubscribed.";
    static const char* MSG_NOT_FOUND = "Email not found in database.";
    static const char* MSG_ERROR = "An error occurred processing your request.";

    char norm[ MAX_EMAIL_LEN + 1 ];
    if (!normalize_email(emailInput, norm, sizeof(norm))) {
        return MSG_INVALID;
    }
    if (!is_valid_email(norm)) {
        return MSG_INVALID;
    }

    int idx = find_index(norm);
    if (idx >= 0) {
        if (remove_at((size_t)idx)) {
            return MSG_OK;
        } else {
            return MSG_ERROR;
        }
    } else {
        return MSG_NOT_FOUND;
    }
}

static int add_to_db(const char* normalized) {
    if (!normalized) return 0;
    if (DB_SIZE >= DB_CAPACITY) return 0;
    // avoid duplicates
    if (find_index(normalized) >= 0) return 1;
    size_t len = strlen(normalized);
    char* dup = (char*)malloc(len + 1);
    if (!dup) return 0;
    memcpy(dup, normalized, len + 1);
    DB[DB_SIZE++] = dup;
    return 1;
}

static int find_index(const char* normalized) {
    if (!normalized) return -1;
    for (size_t i = 0; i < DB_SIZE; i++) {
        if (DB[i] && strcmp(DB[i], normalized) == 0) return (int)i;
    }
    return -1;
}

static int remove_at(size_t idx) {
    if (idx >= DB_SIZE) return 0;
    secure_free(&DB[idx]);
    // compact array
    for (size_t i = idx + 1; i < DB_SIZE; i++) {
        DB[i - 1] = DB[i];
    }
    DB_SIZE--;
    DB[DB_SIZE] = NULL;
    return 1;
}

static void clear_db() {
    for (size_t i = 0; i < DB_SIZE; i++) {
        secure_free(&DB[i]);
    }
    DB_SIZE = 0;
}

static void seed_db() {
    clear_db();
    add_to_db("a@example.com");
    add_to_db("b@example.org");
    add_to_db("c+news@domain.co");
    add_to_db("user@sub.domain.com");
    add_to_db("test@example.com");
}

int main(void) {
    seed_db();
    const char* tests[5] = {
        "a@example.com",
        "  TEST@example.com  ",
        "notfound@nowhere.net",
        "invalid-email",
        "C+News@Domain.Co"
    };
    for (int i = 0; i < 5; i++) {
        const char* res = unsubscribe(tests[i]);
        printf("Input: %s -> %s\n", tests[i], res);
    }
    clear_db();
    return 0;
}