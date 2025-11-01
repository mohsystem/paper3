#include <stdio.h>
#include <string.h>

typedef struct {
    int has_user;
    char username[128];
} Session;

const char* index_page() {
    return "<html><body><a href='/login'>Login</a></body></html>";
}

const char* login_page() {
    return "<html><body><form method='post' action='/do_login'>"
           "Username: <input type='text' name='username'/> "
           "Password: <input type='password' name='password'/> "
           "<button type='submit'>Login</button>"
           "</form></body></html>";
}

int credentials_valid(const char* u, const char* p) {
    return ((strcmp(u, "admin") == 0 && strcmp(p, "password") == 0) ||
            (strcmp(u, "alice") == 0 && strcmp(p, "wonderland") == 0));
}

const char* do_login(const char* username, const char* password, Session* session) {
    if (credentials_valid(username, password)) {
        session->has_user = 1;
        strncpy(session->username, username, sizeof(session->username) - 1);
        session->username[sizeof(session->username) - 1] = '\0';
        return "REDIRECT:/secret";
    } else {
        session->has_user = 0;
        session->username[0] = '\0';
        return "REDIRECT:/login?error=1";
    }
}

const char* secret_page(Session* session) {
    if (session != NULL && session->has_user) {
        return "Secret settings";
    }
    return "REDIRECT:/login";
}

int main() {
    // Test 1: index page
    printf("TEST1 index: %s\n", index_page());
    // Test 2: login page
    printf("TEST2 login page: %s\n", login_page());
    // Test 3: failed login attempt
    Session session1 = {0, ""};
    printf("TEST3 do_login fail: %s\n", do_login("admin", "wrong", &session1));
    printf("TEST3 secret after fail: %s\n", secret_page(&session1));
    // Test 4: successful login (admin)
    Session session2 = {0, ""};
    printf("TEST4 do_login success: %s\n", do_login("admin", "password", &session2));
    printf("TEST4 secret after success: %s\n", secret_page(&session2));
    // Test 5: successful login (alice)
    Session session3 = {0, ""};
    printf("TEST5 do_login success: %s\n", do_login("alice", "wonderland", &session3));
    printf("TEST5 secret after success: %s\n", secret_page(&session3));
    return 0;
}