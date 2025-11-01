
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// GET request - display login form
char* login() {
    static char form[1024];
    snprintf(form, sizeof(form),
        "<html><body>\\n"
        "<h2>Login Form</h2>\\n"
        "<form method='POST' action='/do_login'>\\n"
        "Username: <input type='text' name='username'><br>\\n"
        "Password: <input type='password' name='password'><br>\\n"
        "<input type='submit' value='Login'>\\n"
        "</form>\\n"
        "</body></html>");
    return form;
}

// POST request - process login
char* do_login(const char* username, const char* password) {
    static char response[1024];
    
    if (username != NULL && strlen(username) > 0 && strcmp(username, "admin") == 0) {
        snprintf(response, sizeof(response), "REDIRECT:/admin_page");
    } else {
        snprintf(response, sizeof(response), 
            "<html><body><h3>Login failed or user is not admin</h3></body></html>");
    }
    
    return response;
}

char* admin_page() {
    static char page[1024];
    snprintf(page, sizeof(page), "<html><body><h2>Welcome to Admin Page</h2></body></html>");
    return page;
}

int main() {
    // Test case 1: Admin login
    printf("Test 1 - Admin login:\\n");
    char* result1 = do_login("admin", "password123");
    printf("%s\\n\\n", result1);
    
    // Test case 2: Non-admin login
    printf("Test 2 - Non-admin login:\\n");
    char* result2 = do_login("user", "password123");
    printf("%s\\n\\n", result2);
    
    // Test case 3: Empty username
    printf("Test 3 - Empty username:\\n");
    char* result3 = do_login("", "password123");
    printf("%s\\n\\n", result3);
    
    // Test case 4: NULL username
    printf("Test 4 - NULL username:\\n");
    char* result4 = do_login(NULL, "password123");
    printf("%s\\n\\n", result4);
    
    // Test case 5: Get login form
    printf("Test 5 - Display login form:\\n");
    char* result5 = login();
    printf("%s\\n", result5);
    
    return 0;
}
