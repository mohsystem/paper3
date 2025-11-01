#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Helper to allocate formatted string
static char* strf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (len < 0) { va_end(args2); return NULL; }
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { va_end(args2); return NULL; }
    vsnprintf(buf, (size_t)len + 1, fmt, args2);
    va_end(args2);
    return buf;
}

// GET /login
char* login(const char* query) {
    if (!query) query = "";
    return strf("GET /login -> provide credentials via POST to /do_login. Query=%s", query);
}

// POST /do_login
char* do_login(const char* username, const char* password) {
    if (username && password && strcmp(username, "admin") == 0 && strcmp(password, "secret") == 0) {
        return strf("admin|admin");
    } else if (username && password && strcmp(username, "user") == 0 && strcmp(password, "pass") == 0) {
        return strf("user|user");
    } else {
        return strf("ERROR: Invalid credentials");
    }
}

// GET /
char* index(const char* sessionToken) {
    if (!sessionToken || strncmp(sessionToken, "ERROR", 5) == 0) {
        return strf("Welcome, Guest");
    }
    const char* bar = strchr(sessionToken, '|');
    if (!bar) return strf("Welcome, %s", sessionToken);
    size_t userLen = (size_t)(bar - sessionToken);
    char* user = (char*)malloc(userLen + 1);
    if (!user) return strf("Welcome, Guest");
    memcpy(user, sessionToken, userLen);
    user[userLen] = '\0';
    char* out = strf("Welcome, %s", user);
    free(user);
    return out;
}

// GET /user
char* user_page(const char* sessionToken) {
    if (!sessionToken || strncmp(sessionToken, "ERROR", 5) == 0) return strf("401 Unauthorized");
    const char* bar = strchr(sessionToken, '|');
    if (!bar) return strf("401 Unauthorized");
    size_t userLen = (size_t)(bar - sessionToken);
    const char* role = bar + 1;
    if (strcmp(role, "user") != 0 && strcmp(role, "admin") != 0) return strf("401 Unauthorized");
    char* user = (char*)malloc(userLen + 1);
    if (!user) return strf("401 Unauthorized");
    memcpy(user, sessionToken, userLen);
    user[userLen] = '\0';
    char* out = strf("User Page for %s", user);
    free(user);
    return out;
}

// GET /admin
char* admin_page(const char* sessionToken) {
    if (!sessionToken || strncmp(sessionToken, "ERROR", 5) == 0) return strf("401 Unauthorized");
    const char* bar = strchr(sessionToken, '|');
    if (!bar) return strf("401 Unauthorized");
    size_t userLen = (size_t)(bar - sessionToken);
    const char* role = bar + 1;
    if (strcmp(role, "admin") != 0) return strf("401 Unauthorized");
    char* user = (char*)malloc(userLen + 1);
    if (!user) return strf("401 Unauthorized");
    memcpy(user, sessionToken, userLen);
    user[userLen] = '\0';
    char* out = strf("Admin Page for %s", user);
    free(user);
    return out;
}

int main(void) {
    // 5 test cases
    char* r1 = login("next=/home");
    printf("Test 1: %s\n", r1);
    free(r1);

    char* tokenUser = do_login("user", "pass");
    char* r2 = index(tokenUser);
    printf("Test 2: %s\n", r2);
    free(r2);

    char* r3 = user_page(tokenUser);
    printf("Test 3: %s\n", r3);
    free(r3);

    char* r4 = admin_page(tokenUser);
    printf("Test 4: %s\n", r4);
    free(r4);
    free(tokenUser);

    char* tokenAdmin = do_login("admin", "secret");
    char* r5 = admin_page(tokenAdmin);
    printf("Test 5: %s\n", r5);
    free(r5);
    free(tokenAdmin);

    return 0;
}