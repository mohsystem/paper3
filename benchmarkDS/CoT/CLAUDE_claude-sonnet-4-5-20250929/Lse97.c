
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define SALT_SIZE 16
#define HASH_SIZE SHA256_DIGEST_LENGTH

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t input_length, char* output) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    int i, j;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    int idx = 0;
    for (i = 0; i < input_length; ) {
        char_array_3[0] = i < input_length ? data[i++] : 0;
        char_array_3[1] = i < input_length ? data[i++] : 0;
        char_array_3[2] = i < input_length ? data[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(j = 0; j < 4 && idx < output_length; j++) {
            output[idx++] = base64_chars[char_array_4[j]];
        }
    }
    
    while (idx % 4 != 0) output[idx++] = '=';
    output[idx] = '\\0';
}

void generate_salt(unsigned char* salt) {
    RAND_bytes(salt, SALT_SIZE);
}

void hash_password(const char* password, const unsigned char* salt, char* output) {
    unsigned char hash[HASH_SIZE];
    SHA256_CTX sha256;
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, SALT_SIZE);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, HASH_SIZE, output);
}

int validate_email(const char* email) {
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", REG_EXTENDED);
    if (reti) return 0;
    
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !reti;
}

char* register_user(const char* username, const char* password, const char* first_name,
                   const char* last_name, const char* email, sqlite3* db) {
    static char result[256];
    
    // Input validation
    if (!username || !password || !first_name || !last_name || !email ||
        strlen(username) == 0 || strlen(password) == 0 || strlen(first_name) == 0 ||
        strlen(last_name) == 0 || strlen(email) == 0) {
        strcpy(result, "Error: All fields are required");
        return result;
    }
    
    // Validate email format
    if (!validate_email(email)) {
        strcpy(result, "Error: Invalid email format");
        return result;
    }
    
    // Password strength validation
    if (strlen(password) < 8) {
        strcpy(result, "Error: Password must be at least 8 characters");
        return result;
    }
    
    // Check if username exists
    sqlite3_stmt* stmt;
    const char* check_query = "SELECT username FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, check_query, -1, &stmt, NULL) != SQLITE_OK) {
        strcpy(result, "Error: Database operation failed");
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        strcpy(result, "Error: Username already exists");
        return result;
    }
    sqlite3_finalize(stmt);
    
    // Generate salt and hash password
    unsigned char salt[SALT_SIZE];
    generate_salt(salt);
    
    char hashed_password[128];
    hash_password(password, salt, hashed_password);
    
    char salt_string[64];
    base64_encode(salt, SALT_SIZE, salt_string);
    
    // Insert user into database
    const char* insert_query = 
        "INSERT INTO users (username, password, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, NULL) != SQLITE_OK) {
        strcpy(result, "Error: Database operation failed");
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_string, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, first_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, last_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, email, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        strcpy(result, "Error: Database operation failed");
        return result;
    }
    
    sqlite3_finalize(stmt);
    strcpy(result, "Success: Registration succeeded");
    return result;
}

int main() {
    printf("Registration System Test Cases:\\n\\n");
    
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    // Create users table
    const char* create_table = 
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL)";
    
    sqlite3_exec(db, create_table, NULL, NULL, NULL);
    
    // Test Case 1: Valid registration
    printf("Test Case 1 - Valid registration:\\n");
    char* result = register_user("john_doe", "SecurePass123", "John", "Doe", "john@example.com", db);
    printf("Result: %s\\n\\n", result);
    
    // Test Case 2: Duplicate username
    printf("Test Case 2 - Duplicate username:\\n");
    result = register_user("john_doe", "Pass456", "Jane", "Smith", "jane@example.com", db);
    printf("Result: %s\\n\\n", result);
    
    // Test Case 3: Empty fields
    printf("Test Case 3 - Empty fields:\\n");
    result = register_user("", "Pass123", "Test", "User", "test@example.com", db);
    printf("Result: %s\\n\\n", result);
    
    // Test Case 4: Invalid email
    printf("Test Case 4 - Invalid email:\\n");
    result = register_user("testuser", "Pass123", "Test", "User", "invalid-email", db);
    printf("Result: %s\\n\\n", result);
    
    // Test Case 5: Weak password
    printf("Test Case 5 - Weak password:\\n");
    result = register_user("testuser2", "123", "Test", "User", "test2@example.com", db);
    printf("Result: %s\\n\\n", result);
    
    sqlite3_close(db);
    return 0;
}
