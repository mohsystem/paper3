#include <stdio.h>
#include <string.h>

static int constant_time_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t max = la > lb ? la : lb;
    unsigned char result = 0;
    for (size_t i = 0; i < max; ++i) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        result |= (unsigned char)(ca ^ cb);
    }
    return (result == 0) && (la == lb);
}

const char* login() {
    return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Login</title></head>"
           "<body><form method=\"POST\" action=\"/do_login\">"
           "<label>Username: <input type=\"text\" name=\"username\" required></label><br>"
           "<label>Password: <input type=\"password\" name=\"password\" required></label><br>"
           "<button type=\"submit\">Login</button>"
           "</form></body></html>";
}

const char* do_login(const char* username, const char* password) {
    (void)password; // not used for admin check per specification
    static const char* REDIRECT = "REDIRECT:/admin_page";
    static const char* EMPTY = "";
    if (username == NULL) username = "";
    if (constant_time_equals(username, "admin")) {
        return REDIRECT;
    }
    return EMPTY;
}

int main(void) {
    printf("GET /login ->\n%s\n\n", login());

    // 5 test cases
    printf("POST /do_login admin/adminpass -> %s\n", do_login("admin", "adminpass"));
    printf("POST /do_login admin/wrong -> %s\n", do_login("admin", "wrong"));
    printf("POST /do_login user/adminpass -> %s\n", do_login("user", "adminpass"));
    printf("POST /do_login empty/empty -> %s\n", do_login("", ""));
    printf("POST /do_login Admin(case)/pass -> %s\n", do_login("Admin", "pass"));
    return 0;
}