#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* username;
    char* email;
    char* ssn;
    int valid;
} Session;

static char* sanitize_user_or_email(const char* v, size_t maxLen) {
    if (!v) {
        char* empty = (char*)calloc(1, 1);
        return empty ? empty : NULL;
    }
    size_t n = strlen(v);
    char* out = (char*)malloc(maxLen + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n && j < maxLen; ++i) {
        unsigned char c = (unsigned char)v[i];
        if (isalnum(c) || c=='@' || c=='.' || c=='-' || c=='_' || c=='+' || c==' ') {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
    return out;
}

static char* sanitize_ssn(const char* v, size_t maxLen) {
    if (!v) {
        char* empty = (char*)calloc(1, 1);
        return empty ? empty : NULL;
    }
    size_t n = strlen(v);
    char* out = (char*)malloc(maxLen + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n && j < maxLen; ++i) {
        unsigned char c = (unsigned char)v[i];
        if (isdigit(c) || c=='-') {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
    return out;
}

// info() function: accepts a session and returns a newly allocated string
char* info(const Session* session) {
    if (session == NULL || !session->valid) {
        char* s = (char*)malloc(13);
        if (!s) return NULL;
        memcpy(s, "Unauthorized", 12);
        s[12] = '\0';
        return s;
    }
    char* u = sanitize_user_or_email(session->username, 100);
    char* e = sanitize_user_or_email(session->email, 254);
    char* s = sanitize_ssn(session->ssn, 11);
    if (!u || !e || !s) {
        free(u); free(e); free(s);
        return NULL;
    }
    const char* prefix1 = "username=";
    const char* mid1 = "; email=";
    const char* mid2 = "; ssn=";
    size_t len = strlen(prefix1) + strlen(u) + strlen(mid1) + strlen(e) + strlen(mid2) + strlen(s) + 1;
    char* out = (char*)malloc(len);
    if (!out) {
        free(u); free(e); free(s);
        return NULL;
    }
    snprintf(out, len, "%s%s%s%s%s%s", prefix1, u, mid1, e, mid2, s);
    free(u); free(e); free(s);
    return out;
}

static void mask_ssn_in_place(char* str) {
    if (!str) return;
    size_t n = strlen(str);
    for (size_t i = 0; i + 10 < n; ++i) {
        if (isdigit((unsigned char)str[i]) &&
            isdigit((unsigned char)str[i+1]) &&
            isdigit((unsigned char)str[i+2]) &&
            str[i+3]=='-' &&
            isdigit((unsigned char)str[i+4]) &&
            isdigit((unsigned char)str[i+5]) &&
            str[i+6]=='-' &&
            isdigit((unsigned char)str[i+7]) &&
            isdigit((unsigned char)str[i+8]) &&
            isdigit((unsigned char)str[i+9]) &&
            isdigit((unsigned char)str[i+10])) {
            str[i]='*'; str[i+1]='*'; str[i+2]='*';
            str[i+4]='*'; str[i+5]='*';
        }
    }
}

int main(void) {
    Session s1 = {(char*)"alice", (char*)"alice@example.com", (char*)"123-45-6789", 1};
    Session s2 = {(char*)"bob", (char*)"bob.smith+dev@example.org", (char*)"000-12-3456", 1};
    Session s3 = {(char*)"charlie_ze", (char*)"charlie.ze@example.net", (char*)"999-99-9999", 1};
    Session s4 = {(char*)"dana-j", (char*)"dana-j@example.co", (char*)"321-54-9876", 1};
    Session invalid = {NULL, NULL, NULL, 0};

    Session* tests[5] = {&s1, &s2, &s3, &s4, &invalid};
    for (int i = 0; i < 5; ++i) {
        char* res = info(tests[i]);
        if (!res) {
            printf("Test %d -> ERROR\n", i+1);
            continue;
        }
        mask_ssn_in_place(res);
        printf("Test %d -> %s\n", i+1, res);
        free(res);
    }
    return 0;
}