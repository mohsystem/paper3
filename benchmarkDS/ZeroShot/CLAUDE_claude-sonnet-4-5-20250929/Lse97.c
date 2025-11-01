
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define DB_NAME "users.db"
#define SALT_LENGTH 16
#define HASH_LENGTH 32

typedef struct {
    int success;
    char message[256];
} RegistrationResult;

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode(const unsigned char* buffer, size_t length) {
    size_t out_len = 4 * ((length + 2) / 3);
    char* result = (char*)malloc(out_len + 1);
    if (!result) return NULL;
    
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(buffer++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                result[j++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        size_t k;
        for(k = i; k < 3; k++)
            char_array_3[k] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (k = 0; k < i + 1; k++)
            result[j++] = base64_chars[char_array_4[k]];
        
        while(i++ < 3)
            result[j++] = '=';
    }
    
    result[j] = '\\0';
    return result;
}

void initialize_database() {
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc) {
        sqlite3_close(db);
        return;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL)";
    
    char* errMsg = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

char* generate_salt() {
    unsigned char salt[SALT_LENGTH];
    RAND_bytes(salt, SALT_LENGTH);
    return base64_encode(salt, SALT_LENGTH);
}

char* hash_password(const char* password, const char* salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

int validate_email(const char* email) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !ret;
}

RegistrationResult register_user(const char* username, const char* password,
                                 const char* first_name, const char* last_name,
                                 const char* email) {
    RegistrationResult result;
    
    if (!username || !password || !first_name || !last_name || !email ||
        strlen(username) == 0 || strlen(password) == 0 || 
        strlen(first_name) == 0 || strlen(last_name) == 0 || 
        strlen(email) == 0) {
        result.success = 0;
        strcpy(result.message, "All fields are required");
        return result;
    }
    
    if (!validate_email(email)) {
        result.success = 0;
        strcpy(result.message, "Invalid email format");
        return result;
    }
    
    initialize_database();
    
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc) {
        sqlite3_close(db);
        result.success = 0;
        strcpy(result.message, "Database error");
        return result;
    }
    
    sqlite3_stmt* stmt;
    const char* check_sql = "SELECT COUNT(*) FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count > 0) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                result.success = 0;
                strcpy(result.message, "Username already exists");
                return result;
            }
        }
        sqlite3_finalize(stmt);
    }
    
    char* salt = generate_salt();
    char* password_hash = hash_password(password, salt);
    
    const char* insert_sql = 
        "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, first_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, last_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, email, -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        free(salt);
        free(password_hash);
        
        if (rc == SQLITE_DONE) {
            sqlite3_close(db);
            result.success = 1;
            strcpy(result.message, "Registration succeeded");
            return result;
        }
    }
    
    free(salt);
    free(password_hash);
    sqlite3_close(db);
    result.success = 0;
    strcpy(result.message, "Database error");
    return result;
}

int main() {
    printf("=== User Registration System Tests ===\\n\\n");
    
    RegistrationResult result1 = register_user("john_doe", "SecurePass123!", 
                                               "John", "Doe", "john@example.com");
    printf("Test 1: %s\\n", result1.message);
    
    RegistrationResult result2 = register_user("jane_smith", "StrongP@ssw0rd", 
                                               "Jane", "Smith", "jane@example.com");
    printf("Test 2: %s\\n", result2.message);
    
    RegistrationResult result3 = register_user("john_doe", "AnotherPass456", 
                                               "John", "Different", "different@example.com");
    printf("Test 3: %s\\n", result3.message);
    
    RegistrationResult result4 = register_user("bob_wilson", "MyP@ss123", 
                                               "Bob", "Wilson", "invalid-email");
    printf("Test 4: %s\\n", result4.message);
    
    RegistrationResult result5 = register_user("alice_brown", "", 
                                               "Alice", "Brown", "alice@example.com");
    printf("Test 5: %s\\n", result5.message);
    
    return 0;
}
