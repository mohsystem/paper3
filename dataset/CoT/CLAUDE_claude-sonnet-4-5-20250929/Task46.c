
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <time.h>
#include <ctype.h>

#define DB_NAME "users.db"
#define MAX_NAME_LEN 101
#define MAX_EMAIL_LEN 256
#define MAX_PASSWORD_LEN 129
#define SALT_LEN 33
#define HASH_LEN 65

typedef struct {
    int success;
    char message[256];
} RegistrationResult;

void generate_salt(char* salt) {
    srand(time(NULL));
    for (int i = 0; i < SALT_LEN - 1; i++) {
        sprintf(&salt[i * 2], "%02x", rand() % 256);
    }
    salt[SALT_LEN - 1] = '\\0';
}

void hash_password(const char* password, const char* salt, char* output) {
    char salted[512];
    snprintf(salted, sizeof(salted), "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)salted, strlen(salted), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&output[i * 2], "%02x", hash[i]);
    }
    output[HASH_LEN - 1] = '\\0';
}

int validate_name(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    return len >= 2 && len <= 100;
}

int validate_email(const char* email) {
    if (email == NULL) return 0;
    size_t len = strlen(email);
    if (len < 3 || len > 254) return 0;
    
    int at_count = 0;
    int dot_after_at = 0;
    int at_pos = -1;
    
    for (size_t i = 0; i < len; i++) {
        if (email[i] == '@') {
            at_count++;
            at_pos = i;
        }
        if (at_count == 1 && email[i] == '.' && i > at_pos) {
            dot_after_at = 1;
        }
    }
    
    return at_count == 1 && dot_after_at && at_pos > 0 && at_pos < len - 1;
}

int validate_password(const char* password) {
    if (password == NULL) return 0;
    size_t len = strlen(password);
    return len >= 8 && len <= 128;
}

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && isspace(*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\\0';
}

void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void initialize_database(void) {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
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
                     "salt TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

RegistrationResult register_user(const char* name, const char* email, const char* password) {
    RegistrationResult result = {0, ""};
    
    // Input validation
    if (!validate_name(name)) {
        strcpy(result.message, "Invalid name. Must be 2-100 characters.");
        return result;
    }
    
    if (!validate_email(email)) {
        strcpy(result.message, "Invalid email format.");
        return result;
    }
    
    if (!validate_password(password)) {
        strcpy(result.message, "Invalid password. Must be 8-128 characters.");
        return result;
    }
    
    // Generate salt and hash password
    char salt[SALT_LEN];
    char password_hash[HASH_LEN];
    generate_salt(salt);
    hash_password(password, salt, password_hash);
    
    // Prepare data
    char trimmed_name[MAX_NAME_LEN];
    char trimmed_email[MAX_EMAIL_LEN];
    strncpy(trimmed_name, name, MAX_NAME_LEN - 1);
    strncpy(trimmed_email, email, MAX_EMAIL_LEN - 1);
    trim(trimmed_name);
    trim(trimmed_email);
    to_lower(trimmed_email);
    
    // Store in database
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        strcpy(result.message, "Database connection failed.");
        return result;
    }
    
    const char* sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        strcpy(result.message, "Failed to prepare statement.");
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, trimmed_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, trimmed_email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, salt, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc == SQLITE_DONE) {
        result.success = 1;
        strcpy(result.message, "User registered successfully.");
    } else if (rc == SQLITE_CONSTRAINT) {
        strcpy(result.message, "Email already registered.");
    } else {
        strcpy(result.message, "Registration failed.");
    }
    
    return result;
}

int main(void) {
    initialize_database();
    
    printf("=== User Registration System Test Cases ===\\n\\n");
    
    // Test Case 1: Valid registration
    RegistrationResult result1 = register_user("John Doe", "john.doe@example.com", "SecurePass123");
    printf("Test 1 - Valid registration: %s\\n", result1.message);
    
    // Test Case 2: Duplicate email
    RegistrationResult result2 = register_user("Jane Doe", "john.doe@example.com", "AnotherPass456");
    printf("Test 2 - Duplicate email: %s\\n", result2.message);
    
    // Test Case 3: Invalid email format
    RegistrationResult result3 = register_user("Bob Smith", "invalid-email", "Password789");
    printf("Test 3 - Invalid email: %s\\n", result3.message);
    
    // Test Case 4: Weak password
    RegistrationResult result4 = register_user("Alice Brown", "alice@example.com", "123");
    printf("Test 4 - Weak password: %s\\n", result4.message);
    
    // Test Case 5: Invalid name
    RegistrationResult result5 = register_user("A", "test@example.com", "ValidPass123");
    printf("Test 5 - Invalid name: %s\\n", result5.message);
    
    return 0;
}
