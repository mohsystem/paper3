#include <stdio.h>
#include <string.h>
#include <regex.h>

#define MAX_USERNAME_LEN 32
#define MIN_USERNAME_LEN 3
#define MAX_EMAIL_LEN 320
#define MAX_PHONE_LEN 16
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128
#define MAX_ZIP_LEN 10

static int safe_fullmatch(const char* pattern, const char* s, size_t maxLen) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len > maxLen) return 0;

    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB);
    if (rc != 0) {
        return 0;
    }
    rc = regexec(&re, s, 0, NULL, 0);
    regfree(&re);
    return rc == 0 ? 1 : 0;
}

int validate_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return 0;
    const char* pattern = "^[A-Za-z0-9_]{3,16}$";
    return safe_fullmatch(pattern, username, MAX_USERNAME_LEN);
}

int validate_email(const char* email) {
    if (email == NULL) return 0;
    if (strlen(email) > MAX_EMAIL_LEN) return 0;
    const char* pattern = "^[A-Za-z0-9_%+.-]+(\\.[A-Za-z0-9_%+.-]+)*@[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)*\\.[A-Za-z]{2,24}$";
    return safe_fullmatch(pattern, email, MAX_EMAIL_LEN);
}

int validate_phone_e164(const char* phone) {
    if (phone == NULL) return 0;
    const char* pattern = "^\\+[1-9][0-9]{1,14}$";
    return safe_fullmatch(pattern, phone, MAX_PHONE_LEN);
}

static int re_search(const char* pattern, const char* s) {
    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        return 0;
    }
    rc = regexec(&re, s, 0, NULL, 0);
    regfree(&re);
    return rc == 0 ? 1 : 0;
}

int validate_password(const char* password) {
    if (password == NULL) return 0;
    size_t len = strlen(password);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return 0;

    if (!re_search("[A-Z]", password)) return 0;
    if (!re_search("[a-z]", password)) return 0;
    if (!re_search("[0-9]", password)) return 0;
    if (!re_search("[^A-Za-z0-9]", password)) return 0;
    return 1;
}

int validate_us_zip(const char* zip) {
    if (zip == NULL) return 0;
    const char* pattern = "^[0-9]{5}(-[0-9]{4})?$";
    return safe_fullmatch(pattern, zip, MAX_ZIP_LEN);
}

int main(void) {
    // 5 test cases
    printf("%s\n", validate_username("user_name12") ? "true" : "false");
    printf("%s\n", validate_email("john.doe@example.co.uk") ? "true" : "false");
    printf("%s\n", validate_phone_e164("+14155552671") ? "true" : "false");
    printf("%s\n", validate_password("Str0ng!Pass") ? "true" : "false");
    printf("%s\n", validate_us_zip("12345-6789") ? "true" : "false");
    return 0;
}