#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define USERNAME_MAX 64

typedef struct {
    int authenticated;
    char username[USERNAME_MAX];
} Session;

static const char* EXPECTED_USER = "admin";
static const char* EXPECTED_PASS = "S3cureP@ss";

char* sanitize(const char* s) {
    if (s == NULL) {
        char* out = (char*)calloc(1, 1);
        return out; // empty string
    }
    char* out = (char*)malloc(USERNAME_MAX + 1);
    if (!out) exit(1);
    size_t count = 0;
    for (size_t i = 0; s[i] != '\0' && count < USERNAME_MAX; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 32 && c <= 126) {
            out[count++] = (char)c;
        }
    }
    out[count] = '\0';
    // trim both ends (spaces only)
    size_t start = 0;
    while (out[start] == ' ') start++;
    size_t end = strlen(out);
    while (end > start && out[end - 1] == ' ') end--;
    size_t len = end - start;
    memmove(out, out + start, len);
    out[len] = '\0';
    return out;
}

int constant_time_equals(const char* a, const char* b) {
    size_t la = a ? strlen(a) : 0;
    size_t lb = b ? strlen(b) : 0;
    size_t max = la > lb ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < max; i++) {
        unsigned char ac = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char bc = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ac ^ bc);
    }
    return diff == 0;
}

char* index_page(Session* session) {
    (void)session;
    const char* body = "<html><body><h1>Index</h1><a href=\"/login\">Login</a></body></html>";
    char* out = (char*)malloc(strlen(body) + 1);
    if (!out) exit(1);
    strcpy(out, body);
    return out;
}

char* login_page() {
    const char* body = "<html><body><h1>Login</h1>"
                       "<form method=\"POST\" action=\"/do_login\">"
                       "<input type=\"text\" name=\"username\" maxlength=\"64\"/>"
                       "<input type=\"password\" name=\"password\" maxlength=\"64\"/>"
                       "<input type=\"submit\" value=\"Login\"/>"
                       "</form></body></html>";
    char* out = (char*)malloc(strlen(body) + 1);
    if (!out) exit(1);
    strcpy(out, body);
    return out;
}

char* do_login(const char* username_in, const char* password_in, Session* session) {
    char* u = sanitize(username_in);
    char* p = sanitize(password_in);
    const char* redirect_fail = "REDIRECT:/login?error=1";
    const char* redirect_ok = "REDIRECT:/secret";
    char* out = NULL;
    if (u[0] == '\0' || p[0] == '\0') {
        out = (char*)malloc(strlen(redirect_fail) + 1);
        if (!out) exit(1);
        strcpy(out, redirect_fail);
        free(u);
        free(p);
        return out;
    }
    int okUser = constant_time_equals(u, EXPECTED_USER);
    int okPass = constant_time_equals(p, EXPECTED_PASS);
    if (okUser && okPass) {
        session->authenticated = 1;
        // safe copy
        size_t len = strlen(u);
        if (len >= USERNAME_MAX) len = USERNAME_MAX - 1;
        memcpy(session->username, u, len);
        session->username[len] = '\0';
        out = (char*)malloc(strlen(redirect_ok) + 1);
        if (!out) exit(1);
        strcpy(out, redirect_ok);
    } else {
        out = (char*)malloc(strlen(redirect_fail) + 1);
        if (!out) exit(1);
        strcpy(out, redirect_fail);
    }
    free(u);
    free(p);
    return out;
}

char* secret_page(Session* session) {
    if (session != NULL && session->authenticated && session->username[0] != '\0') {
        const char* body = "<html><body>Secret settings</body></html>";
        char* out = (char*)malloc(strlen(body) + 1);
        if (!out) exit(1);
        strcpy(out, body);
        return out;
    } else {
        const char* redir = "REDIRECT:/login";
        char* out = (char*)malloc(strlen(redir) + 1);
        if (!out) exit(1);
        strcpy(out, redir);
        return out;
    }
}

int main(void) {
    // Test case 1: Index page
    Session s1 = {0, {0}};
    char* res1 = index_page(&s1);
    printf("C TC1 INDEX: %s\n", res1);
    free(res1);

    // Test case 2: Secret without login
    Session s2 = {0, {0}};
    char* res2 = secret_page(&s2);
    printf("C TC2 SECRET (no login): %s\n", res2);
    free(res2);

    // Test case 3: Wrong password
    Session s3 = {0, {0}};
    char* res3 = do_login("admin", "wrong", &s3);
    printf("C TC3 LOGIN wrong: %s\n", res3);
    free(res3);
    char* res3b = secret_page(&s3);
    printf("C TC3 SECRET after wrong: %s\n", res3b);
    free(res3b);

    // Test case 4: Correct login then secret
    Session s4 = {0, {0}};
    char* res4 = do_login("admin", "S3cureP@ss", &s4);
    printf("C TC4 LOGIN correct: %s\n", res4);
    free(res4);
    char* res4b = secret_page(&s4);
    printf("C TC4 SECRET after correct: %s\n", res4b);
    free(res4b);

    // Test case 5: Empty credentials
    Session s5 = {0, {0}};
    char* res5 = do_login("", "", &s5);
    printf("C TC5 LOGIN empty: %s\n", res5);
    free(res5);
    char* res5b = secret_page(&s5);
    printf("C TC5 SECRET after empty: %s\n", res5b);
    free(res5b);

    return 0;
}