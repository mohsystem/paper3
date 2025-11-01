#include <stdio.h>
#include <string.h>

const char* login() {
    return "<!DOCTYPE html>\n"
           "<html>\n"
           "<head><title>Login</title></head>\n"
           "<body>\n"
           "  <h1>Login</h1>\n"
           "  <form method=\"POST\" action=\"/do_login\">\n"
           "    <label>Username: <input type=\"text\" name=\"username\"/></label><br/>\n"
           "    <label>Password: <input type=\"password\" name=\"password\"/></label><br/>\n"
           "    <button type=\"submit\">Login</button>\n"
           "  </form>\n"
           "</body>\n"
           "</html>";
}

const char* admin_page() {
    return "<!DOCTYPE html>\n"
           "<html>\n"
           "<head><title>Admin</title></head>\n"
           "<body><h1>Welcome, Admin!</h1></body>\n"
           "</html>";
}

const char* do_login(const char* username, const char* password) {
    if (username != NULL && strcmp(username, "admin") == 0) {
        return "REDIRECT:/admin_page";
    }
    return login();
}

int main() {
    // Test 1: GET /login
    printf("Test 1 - GET /login:\n");
    printf("%s\n", login());
    printf("-----\n");

    // Test 2: POST /do_login with admin user
    printf("Test 2 - POST /do_login (admin, 1234):\n");
    const char* res2 = do_login("admin", "1234");
    printf("%s\n", res2);
    if (strncmp(res2, "REDIRECT:/admin_page", 20) == 0) {
        printf("%s\n", admin_page());
    }
    printf("-----\n");

    // Test 3: POST /do_login with non-admin user
    printf("Test 3 - POST /do_login (user, pass):\n");
    const char* res3 = do_login("user", "pass");
    printf("%s\n", res3);
    printf("-----\n");

    // Test 4: POST /do_login with Admin (case-sensitive fail)
    printf("Test 4 - POST /do_login (Admin, 1234):\n");
    const char* res4 = do_login("Admin", "1234");
    printf("%s\n", res4);
    printf("-----\n");

    // Test 5: POST /do_login with admin and empty password
    printf("Test 5 - POST /do_login (admin, ''):\n");
    const char* res5 = do_login("admin", "");
    printf("%s\n", res5);
    if (strncmp(res5, "REDIRECT:/admin_page", 20) == 0) {
        printf("%s\n", admin_page());
    }
    printf("-----\n");

    return 0;
}