
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* login_form() {
    static char form[1024];
    snprintf(form, sizeof(form),
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head><title>Login</title></head>\\n"
        "<body>\\n"
        "    <h2>Login Form</h2>\\n"
        "    <form action='/do_login' method='post'>\\n"
        "        Username: <input type='text' name='username' required><br><br>\\n"
        "        Password: <input type='password' name='password' required><br><br>\\n"
        "        <input type='submit' value='Login'>\\n"
        "    </form>\\n"
        "</body>\\n"
        "</html>");
    return form;
}

int is_admin(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    return (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0);
}

const char* do_login(const char* username, const char* password) {
    if (username != NULL && password != NULL && strlen(username) > 0 && strlen(password) > 0) {
        if (is_admin(username, password)) {
            return "redirect:/admin_page";
        } else {
            return "<h3>Access Denied</h3><a href='/login'>Try Again</a>";
        }
    }
    return "<h3>Invalid input</h3><a href='/login'>Try Again</a>";
}

int main() {
    printf("Test Case 1: Valid admin credentials\\n");
    printf("Result: %s\\n", do_login("admin", "admin123"));
    
    printf("\\nTest Case 2: Invalid username\\n");
    printf("Result: %s\\n", do_login("user", "admin123"));
    
    printf("\\nTest Case 3: Invalid password\\n");
    printf("Result: %s\\n", do_login("admin", "wrongpass"));
    
    printf("\\nTest Case 4: Empty credentials\\n");
    printf("Result: %s\\n", do_login("", ""));
    
    printf("\\nTest Case 5: NULL values\\n");
    printf("Result: %s\\n", do_login(NULL, NULL));
    
    return 0;
}
