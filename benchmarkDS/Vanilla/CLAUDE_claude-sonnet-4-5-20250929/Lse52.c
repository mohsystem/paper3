
// C web application using CGI approach
// This is a simplified demonstration

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[100];
    int authenticated;
} Session;

Session global_session = {"", 0};

void show_index_page() {
    printf("<html><body>\\n");
    printf("<h1>Welcome to Index Page</h1>\\n");
    printf("<a href='/login'>Go to Login</a>\\n");
    printf("</body></html>\\n");
}

void show_login_page() {
    printf("<html><body>\\n");
    printf("<h1>Login Page</h1>\\n");
    printf("<form action='/do_login' method='post'>\\n");
    printf("Username: <input type='text' name='username'><br>\\n");
    printf("Password: <input type='password' name='password'><br>\\n");
    printf("<input type='submit' value='Login'>\\n");
    printf("</form>\\n");
    printf("</body></html>\\n");
}

int do_login(const char* username, const char* password) {
    if (strcmp(username, "admin") == 0 && strcmp(password, "password123") == 0) {
        strcpy(global_session.username, username);
        global_session.authenticated = 1;
        return 1;
    }
    return 0;
}

void show_secret_page() {
    if (global_session.authenticated) {
        printf("<html><body>\\n");
        printf("<h1>Secret settings</h1>\\n");
        printf("</body></html>\\n");
    } else {
        printf("<html><body>\\n");
        printf("<h1>Access Denied</h1>\\n");
        printf("<a href='/login'>Please login</a>\\n");
        printf("</body></html>\\n");
    }
}

void handle_request(const char* path, const char* method, 
                   const char* username, const char* password) {
    if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        show_index_page();
    } else if (strcmp(path, "/login") == 0 && strcmp(method, "GET") == 0) {
        show_login_page();
    } else if (strcmp(path, "/do_login") == 0 && strcmp(method, "POST") == 0) {
        if (do_login(username, password)) {
            printf("Redirect to /secret\\n");
        } else {
            printf("Redirect to /login\\n");
        }
    } else if (strcmp(path, "/secret") == 0 && strcmp(method, "GET") == 0) {
        show_secret_page();
    }
}

int main() {
    printf("Test Case 1: Access Index Page\\n");
    handle_request("/", "GET", "", "");
    printf("\\n");
    
    printf("Test Case 2: Access Login Page\\n");
    handle_request("/login", "GET", "", "");
    printf("\\n");
    
    printf("Test Case 3: Valid Login\\n");
    handle_request("/do_login", "POST", "admin", "password123");
    printf("\\n");
    
    printf("Test Case 4: Access Secret Page (Authenticated)\\n");
    handle_request("/secret", "GET", "", "");
    printf("\\n");
    
    printf("Test Case 5: Access Secret Page (Not Authenticated)\\n");
    global_session.authenticated = 0;
    strcpy(global_session.username, "");
    handle_request("/secret", "GET", "", "");
    printf("\\n");
    
    return 0;
}
