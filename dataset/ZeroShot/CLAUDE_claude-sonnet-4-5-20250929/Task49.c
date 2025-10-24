
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>
#include <ctype.h>

#define DB_NAME "users.db"
#define MAX_NAME_LEN 100
#define MAX_EMAIL_LEN 255
#define MAX_MSG_LEN 256

typedef struct {
    int success;
    char message[MAX_MSG_LEN];
} ApiResponse;

void sanitizeInput(char* input) {
    if (input == NULL) return;
    
    char* src = input;
    char* dst = input;
    
    while (*src) {
        if (*src != '<' && *src != '>' && *src != '"' && 
            *src != '\\'' && *src != ';' && *src != '\\0') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\\0';
    
    while (isspace((unsigned char)*input)) input++;
    
    char* end = input + strlen(input) - 1;
    while (end > input && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\\0';
}

int validateEmail(const char* email) {
    regex_t regex;
    int reti;
    
    const char* pattern = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) return 0;
    
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !reti;
}

int validateInput(const char* name, const char* email, int age) {
    if (name == NULL || strlen(name) == 0 || strlen(name) > MAX_NAME_LEN) {
        return 0;
    }
    if (email == NULL || !validateEmail(email)) {
        return 0;
    }
    if (age < 0 || age > 150) {
        return 0;
    }
    return 1;
}

void initDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "email TEXT NOT NULL UNIQUE,"
                     "age INTEGER NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

ApiResponse storeUserData(char* name, char* email, int age) {
    ApiResponse response;
    response.success = 0;
    strcpy(response.message, "Unknown error");
    
    sanitizeInput(name);
    sanitizeInput(email);
    
    if (!validateInput(name, email, age)) {
        response.success = 0;
        strcpy(response.message, "Invalid input data");
        return response;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        strcpy(response.message, "Database connection error");
        return response;
    }
    
    const char* sql = "INSERT INTO users (name, email, age) VALUES (?, ?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        strcpy(response.message, "Statement preparation error");
        sqlite3_close(db);
        return response;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, age);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        response.success = 1;
        strcpy(response.message, "User data stored successfully");
    } else if (rc == SQLITE_CONSTRAINT) {
        response.success = 0;
        strcpy(response.message, "Email already exists");
    } else {
        response.success = 0;
        snprintf(response.message, MAX_MSG_LEN, "Database error: %s", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return response;
}

int main() {
    initDatabase();
    
    printf("=== API Endpoint Test Cases ===\\n\\n");
    
    // Test Case 1: Valid user data
    printf("Test 1 - Valid user:\\n");
    char name1[] = "John Doe";
    char email1[] = "john@example.com";
    ApiResponse response1 = storeUserData(name1, email1, 25);
    printf("Success: %d, Message: %s\\n\\n", response1.success, response1.message);
    
    // Test Case 2: Another valid user
    printf("Test 2 - Another valid user:\\n");
    char name2[] = "Jane Smith";
    char email2[] = "jane@example.com";
    ApiResponse response2 = storeUserData(name2, email2, 30);
    printf("Success: %d, Message: %s\\n\\n", response2.success, response2.message);
    
    // Test Case 3: Duplicate email
    printf("Test 3 - Duplicate email:\\n");
    char name3[] = "John Clone";
    char email3[] = "john@example.com";
    ApiResponse response3 = storeUserData(name3, email3, 35);
    printf("Success: %d, Message: %s\\n\\n", response3.success, response3.message);
    
    // Test Case 4: Invalid email format
    printf("Test 4 - Invalid email:\\n");
    char name4[] = "Invalid User";
    char email4[] = "notanemail";
    ApiResponse response4 = storeUserData(name4, email4, 28);
    printf("Success: %d, Message: %s\\n\\n", response4.success, response4.message);
    
    // Test Case 5: Invalid age
    printf("Test 5 - Invalid age:\\n");
    char name5[] = "Old Person";
    char email5[] = "old@example.com";
    ApiResponse response5 = storeUserData(name5, email5, 200);
    printf("Success: %d, Message: %s\\n\\n", response5.success, response5.message);
    
    return 0;
}
