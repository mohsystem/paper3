
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_USERS 10
#define MAX_USERNAME 21
#define MAX_PASSWORD 65
#define SALT_SIZE 17

typedef struct {
    char username[MAX_USERNAME];
    char password_hash[MAX_PASSWORD];
    char salt[SALT_SIZE];
} User;

static User userDatabase[MAX_USERS];
static int userCount = 0;

// Simple hash function (for demonstration - use proper crypto library in production)
void simpleHash(const char* password, const char* salt, char* output) {
    unsigned long hash = 5381;
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    for (int i = 0; combined[i] != '\\0'; i++) {
        hash = ((hash << 5) + hash) + combined[i];
    }
    
    snprintf(output, MAX_PASSWORD, "%016lx", hash);
}

void generateSalt(char* salt) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < SALT_SIZE - 1; i++) {
        salt[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    salt[SALT_SIZE - 1] = '\\0';
}

void initialize() {
    char salt[SALT_SIZE];
    generateSalt(salt);
    
    strcpy(userDatabase[0].username, "admin");
    strcpy(userDatabase[0].salt, salt);
    simpleHash("admin123", salt, userDatabase[0].password_hash);
    userCount = 1;
}

int isValidUsername(const char* username) {
    if (strlen(username) < 3 || strlen(username) > 20) return 0;
    
    for (int i = 0; username[i] != '\\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    return 1;
}

char* login() {
    static char form[512];
    snprintf(form, sizeof(form),
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head><title>Login</title></head>\\n"
        "<body>\\n"
        "<h2>Login Form</h2>\\n"
        "<form method='POST' action='/do_login'>\\n"
        "  <label>Username: <input type='text' name='username' required></label><br>\\n"
        "  <label>Password: <input type='password' name='password' required></label><br>\\n"
        "  <input type='submit' value='Login'>\\n"
        "</form>\\n"
        "</body>\\n"
        "</html>");
    return form;
}

char* doLogin(const char* username, const char* password) {
    static char result[64];
    
    // Input validation
    if (username == NULL || password == NULL || 
        strlen(username) == 0 || strlen(password) == 0) {
        strcpy(result, "Invalid credentials");
        return result;
    }
    
    // Validate username format
    if (!isValidUsername(username)) {
        strcpy(result, "Invalid username format");
        return result;
    }
    
    // Check credentials
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            char hashedPassword[MAX_PASSWORD];
            simpleHash(password, userDatabase[i].salt, hashedPassword);
            
            if (strcmp(hashedPassword, userDatabase[i].password_hash) == 0) {
                if (strcmp(username, "admin") == 0) {
                    strcpy(result, "REDIRECT:/admin_page");
                    return result;
                }
                strcpy(result, "Login successful");
                return result;
            }
        }
    }
    
    strcpy(result, "Invalid credentials");
    return result;
}

int main() {
    initialize();
    
    printf("Test Case 1 - GET login form:\\n");
    printf("%.50s...\\n\\n", login());
    
    printf("Test Case 2 - Valid admin login:\\n");
    printf("%s\\n\\n", doLogin("admin", "admin123"));
    
    printf("Test Case 3 - Invalid password:\\n");
    printf("%s\\n\\n", doLogin("admin", "wrongpass"));
    
    printf("Test Case 4 - Non-existent user:\\n");
    printf("%s\\n\\n", doLogin("user123", "password"));
    
    printf("Test Case 5 - Empty credentials:\\n");
    printf("%s\\n\\n", doLogin("", ""));
    
    return 0;
}
