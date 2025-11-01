
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int insert_user(const char* username, const char* password) {
    char hashed_password[65];
    hash_password(password, hashed_password);
    
    sqlite3* db;
    char* err_msg = 0;
    
    int rc = sqlite3_open("users.db", &db);
    if(rc != SQLITE_OK) {
        printf("Error opening database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    char* create_table_sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
    
    char insert_sql[512];
    sprintf(insert_sql, "INSERT INTO users (username, password) VALUES ('%s', '%s');", username, hashed_password);
    rc = sqlite3_exec(db, insert_sql, 0, 0, &err_msg);
    
    if(rc != SQLITE_OK) {
        printf("Error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

int main() {
    printf("Test Case 1:\\n");
    printf("%s\\n", insert_user("user1", "password123") ? "Success" : "Failed");
    
    printf("\\nTest Case 2:\\n");
    printf("%s\\n", insert_user("user2", "securePass456") ? "Success" : "Failed");
    
    printf("\\nTest Case 3:\\n");
    printf("%s\\n", insert_user("admin", "admin@2024") ? "Success" : "Failed");
    
    printf("\\nTest Case 4:\\n");
    printf("%s\\n", insert_user("testuser", "test1234") ? "Success" : "Failed");
    
    printf("\\nTest Case 5:\\n");
    printf("%s\\n", insert_user("john_doe", "mySecret99") ? "Success" : "Failed");
    
    return 0;
}
