
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <openssl/sha.h>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD "password"
#define DB_NAME "userdb"
#define HASH_LENGTH 65

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int update_password(const char* username, const char* new_password) {
    char hashed_password[HASH_LENGTH];
    hash_password(new_password, hashed_password);
    
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\\n");
        return 0;
    }
    
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "UPDATE users SET password = ? WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    unsigned long hash_length = strlen(hashed_password);
    unsigned long username_length = strlen(username);
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)hashed_password;
    bind[0].buffer_length = hash_length;
    bind[0].length = &hash_length;
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)username;
    bind[1].buffer_length = username_length;
    bind[1].length = &username_length;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    int rows_affected = mysql_stmt_affected_rows(stmt);
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return rows_affected > 0;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    int result1 = update_password("john_doe", "newPassword123");
    printf("Update result for john_doe: %s\\n", result1 ? "true" : "false");
    
    printf("\\nTest Case 2:\\n");
    int result2 = update_password("jane_smith", "SecurePass456!");
    printf("Update result for jane_smith: %s\\n", result2 ? "true" : "false");
    
    printf("\\nTest Case 3:\\n");
    int result3 = update_password("admin", "Admin@2024");
    printf("Update result for admin: %s\\n", result3 ? "true" : "false");
    
    printf("\\nTest Case 4:\\n");
    int result4 = update_password("test_user", "TestPass789#");
    printf("Update result for test_user: %s\\n", result4 ? "true" : "false");
    
    printf("\\nTest Case 5:\\n");
    int result5 = update_password("guest", "GuestPassword2024");
    printf("Update result for guest: %s\\n", result5 ? "true" : "false");
    
    return 0;
}
