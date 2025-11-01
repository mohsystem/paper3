#include <stdio.h>
#include <string.h>
#include <stddef.h>

static size_t safe_strlen(const char* s) {
    if (s == NULL) return 0u;
    return strlen(s);
}

static int constant_time_equals(const char* a, const char* b) {
    size_t la = safe_strlen(a);
    size_t lb = safe_strlen(b);
    size_t max = la > lb ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < max; ++i) {
        unsigned char x = (i < la && a != NULL) ? (unsigned char)a[i] : 0u;
        unsigned char y = (i < lb && b != NULL) ? (unsigned char)b[i] : 0u;
        diff |= (unsigned int)(x ^ y);
    }
    return diff == 0u;
}

const char* check_mod(const char* username, const char* password) {
    static const char* STORED_USER = "admin";
    static const char* STORED_PASS = "S3cureP@ss!";
    int userOk = constant_time_equals(username ? username : "", STORED_USER);
    int passOk = constant_time_equals(password ? password : "", STORED_PASS);
    return (userOk && passOk) ? "true" : "false";
}

int main(void) {
    // 5 test cases
    printf("%s\n", check_mod("admin", "S3cureP@ss!"));   // expected true
    printf("%s\n", check_mod("admin", "wrong"));         // expected false
    printf("%s\n", check_mod("user", "S3cureP@ss!"));    // expected false
    printf("%s\n", check_mod("", ""));                   // expected false
    printf("%s\n", check_mod("admin", "S3cureP@ss!!"));  // expected false
    return 0;
}