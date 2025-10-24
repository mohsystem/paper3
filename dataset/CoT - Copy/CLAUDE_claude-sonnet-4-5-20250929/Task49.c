
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <regex.h>

#define DB_PATH "users.db"
#define MAX_NAME_LEN 100
#define MAX_EMAIL_LEN 255
#define MIN_PASSWORD_LEN 8
#define HASH_SIZE 65

typedef struct {
    int success;
    char message[256];
} APIResponse;

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int validate_email(const char* email) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", 
                     REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

int validate_name(const char* name) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z[:space:]]{1,100}$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, name, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

int validate_input(const char* name, const char* email, const char* password) {
    if (!name || !email || !password) return 0;
    if (strlen(name) > MAX_NAME_LEN) return 0;
    if (strlen(email) > MAX_EMAIL_LEN) return 0;
    if (strlen(password) < MIN_PASSWORD_LEN) return 0;
    if (!validate_name(name)) return 0;
    if (!validate_email(email)) return 0;
    
    return 1;
}

void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "email TEXT UNIQUE NOT NULL,"
                     "password_hash TEXT NOT NULL,"
                     "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

APIResponse create_user(const char* name, const char* email, const char* password) {
    APIResponse response;
    
    if (!validate_input(name, email, password)) {
        response.success = 0;
        strcpy(response.message, "Invalid input data");
        return response;
    }
    
    char password_hash[HASH_SIZE];
    hash_password(password, password_hash);
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        response.success = 0;
        strcpy(response.message, "Database connection error");
        return response;
    }
    
    const char* sql = "INSERT INTO users (name, email, password_hash) VALUES (?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response.success = 0;
        strcpy(response.message, "Statement preparation error");
        sqlite3_close(db);
        return response;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password_hash, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc == SQLITE_DONE) {
        response.success = 1;
        strcpy(response.message, "User created successfully");
    } else if (rc == SQLITE_CONSTRAINT) {
        response.success = 0;
        strcpy(response.message, "Email already exists");
    } else {
        response.success = 0;
        strcpy(response.message, "Database error");
    }
    
    return response;
}

int main() {
    initialize_database();
    
    printf("=== Test Case 1: Valid user ===\\n");
    APIResponse resp1 = create_user("John Doe", "john@example.com", "SecurePass123");
    printf("Success: %d, Message: %s\\n", resp1.success, resp1.message);
    
    printf("\\n=== Test Case 2: Duplicate email ===\\n");
    APIResponse resp2 = create_user("Jane Doe", "john@example.com", "AnotherPass456");
    printf("Success: %d, Message: %s\\n", resp2.success, resp2.message);
    
    printf("\\n=== Test Case 3: Invalid email ===\\n");
    APIResponse resp3 = create_user("Alice Smith", "invalid-email", "Password789");
    printf("Success: %d, Message: %s\\n", resp3.success, resp3.message);
    
    printf("\\n=== Test Case 4: Short password ===\\n");
    APIResponse resp4 = create_user("Bob Johnson", "bob@example.com", "short");
    printf("Success: %d, Message: %s\\n", resp4.success, resp4.message);
    
    printf("\\n=== Test Case 5: Invalid name with numbers ===\\n");
    APIResponse resp5 = create_user("Charlie123", "charlie@example.com", "ValidPass123");
    printf("Success: %d, Message: %s\\n", resp5.success, resp5.message);
    
    return 0;
}
