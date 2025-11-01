#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char email[255];
    char password[129];
    int loggedIn; /* use int for portability */
} User;

static size_t my_strnlen_c(const char* s, size_t maxlen) {
    if (!s) return 0;
    size_t i = 0;
    for (; i < maxlen; ++i) {
        if (s[i] == '\0') break;
    }
    return i;
}

static int safe_copy(char* dst, size_t dstsz, const char* src) {
    if (!dst || dstsz == 0) return 0;
    if (!src) { dst[0] = '\0'; return 0; }
    size_t n = my_strnlen_c(src, dstsz - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
    return (my_strnlen_c(src, (size_t)-1) == n) ? 1 : 0;
}

static int ct_equal_strings(const char* a, const char* b, size_t maxCheck) {
    size_t la = my_strnlen_c(a, maxCheck);
    size_t lb = my_strnlen_c(b, maxCheck);
    size_t maxl = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < maxl; ++i) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ca ^ cb);
    }
    return diff == 0;
}

User init_user(const char* email, const char* password) {
    User u;
    memset(&u, 0, sizeof(u));
    u.loggedIn = 0;
    safe_copy(u.email, sizeof(u.email), email ? email : "");
    safe_copy(u.password, sizeof(u.password), password ? password : "");
    return u;
}

int login(User* u, const char* email, const char* password) {
    if (!u || !email || !password) return 0;
    if (strncmp(u->email, email, sizeof(u->email)) != 0) return 0;
    if (!ct_equal_strings(u->password, password, sizeof(u->password))) return 0;
    u->loggedIn = 1;
    return 1;
}

void logout(User* u) {
    if (u) u->loggedIn = 0;
}

int change_email(User* u, const char* oldEmail, const char* password, const char* newEmail) {
    if (!u || !oldEmail || !password || !newEmail) return 0;
    if (!u->loggedIn) return 0;
    if (strncmp(u->email, oldEmail, sizeof(u->email)) != 0) return 0;
    if (!ct_equal_strings(u->password, password, sizeof(u->password))) return 0;
    if (my_strnlen_c(newEmail, sizeof(u->email) - 1) == sizeof(u->email) - 1 &&
        newEmail[sizeof(u->email) - 1] != '\0') {
        return 0; /* too long, would truncate */
    }
    safe_copy(u->email, sizeof(u->email), newEmail);
    return 1;
}

const char* get_email(const User* u) {
    if (!u) return "";
    return u->email;
}

int main(void) {
    /* Test Case 1: Successful login and email change */
    User u1 = init_user("user@example.com", "StrongPass!123");
    int t1Login = login(&u1, "user@example.com", "StrongPass!123");
    int t1Change = change_email(&u1, "user@example.com", "StrongPass!123", "new1@example.com");
    printf("TC1 login: %s, change: %s, email: %s\n", t1Login ? "true" : "false", t1Change ? "true" : "false", get_email(&u1));

    /* Test Case 2: Attempt change without login */
    User u2 = init_user("alpha@ex.com", "Ap@55word");
    int t2Change = change_email(&u2, "alpha@ex.com", "Ap@55word", "alpha2@ex.com");
    printf("TC2 change without login: %s, email: %s\n", t2Change ? "true" : "false", get_email(&u2));

    /* Test Case 3: Wrong old email after login */
    User u3 = init_user("beta@ex.com", "BetaSecure1!");
    int t3Login = login(&u3, "beta@ex.com", "BetaSecure1!");
    int t3Change = change_email(&u3, "wrong@ex.com", "BetaSecure1!", "beta2@ex.com");
    printf("TC3 login: %s, change with wrong old email: %s, email: %s\n", t3Login ? "true" : "false", t3Change ? "true" : "false", get_email(&u3));

    /* Test Case 4: Wrong password after login */
    User u4 = init_user("gamma@ex.com", "GammaStrong1#");
    int t4Login = login(&u4, "gamma@ex.com", "GammaStrong1#");
    int t4Change = change_email(&u4, "gamma@ex.com", "WrongPass", "gamma2@ex.com");
    printf("TC4 login: %s, change with wrong password: %s, email: %s\n", t4Login ? "true" : "false", t4Change ? "true" : "false", get_email(&u4));

    /* Test Case 5: Change email again after re-login with updated email */
    logout(&u1);
    int t5Login = login(&u1, "new1@example.com", "StrongPass!123");
    int t5Change = change_email(&u1, "new1@example.com", "StrongPass!123", "final@example.com");
    printf("TC5 login with updated email: %s, change: %s, email: %s\n", t5Login ? "true" : "false", t5Change ? "true" : "false", get_email(&u1));

    return 0;
}