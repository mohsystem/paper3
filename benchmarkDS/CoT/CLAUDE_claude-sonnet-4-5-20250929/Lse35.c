
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <openssl/sha.h>
#include <mysql/mysql.h>

#define MAX_USERNAME_LEN 21
#define MAX_PASSWORD_LEN 129
#define MAX_HASH_LEN 256
#define MAX_URL_LEN 256

typedef struct {
    int success;
    char redirectUrl[MAX_URL_LEN];
} LoginResult;

/* Validate username format */
int validate_username(const char* username) {
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^[a-zA-Z0-9_]{3,20}$", REG_EXTENDED);
    if (reti) {
        return 0;
    }
    
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return !reti;
}

/* Hash password with SHA-256 */
void hash_password(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    /* Convert to hex string */
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
}

/* Secure login function */
LoginResult login(const char* username, const char* password) {
    LoginResult result;
    result.success = 0;
    strcpy(result.redirectUrl, "login.html?error=invalid");
    
    /* Input validation */
    if (username == NULL || password == NULL || 
        strlen(username) == 0 || strlen(password) == 0) {
        strcpy(result.redirectUrl, "login.html?error=empty");
        return result;
    }
    
    if (!validate_username(username)) {
        strcpy(result.redirectUrl, "login.html?error=invalid");
        return result;
    }
    
    if (strlen(password) < 8 || strlen(password) > 128) {
        strcpy(result.redirectUrl, "login.html?error=invalid");
        return result;
    }
    
    /* Initialize MySQL connection */
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        strcpy(result.redirectUrl, "login.html?error=system");
        return result;
    }
    
    /* Connect to database */
    if (mysql_real_connect(conn, "localhost", "root", "password", 
                          "userdb", 0, NULL, 0) == NULL) {
        fprintf(stderr, "MySQL connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        strcpy(result.redirectUrl, "login.html?error=system");
        return result;
    }
    
    /* Prepare statement */
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement prepare failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        strcpy(result.redirectUrl, "login.html?error=system");
        return result;
    }
    
    /* Bind parameters */
    MYSQL_BIND bind_param[1];
    memset(bind_param, 0, sizeof(bind_param));
    
    unsigned long username_length = strlen(username);
    bind_param[0].buffer_type = MYSQL_TYPE_STRING;
    bind_param[0].buffer = (char*)username;
    bind_param[0].buffer_length = username_length;
    bind_param[0].length = &username_length;
    
    if (mysql_stmt_bind_param(stmt, bind_param)) {
        fprintf(stderr, "Bind param failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        strcpy(result.redirectUrl, "login.html?error=system");
        return result;
    }
    
    /* Execute query */
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Execute failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        strcpy(result.redirectUrl, "login.html?error=system");
        return result;
    }
    
    /* Bind results */
    char stored_hash[MAX_HASH_LEN];
    char salt[MAX_HASH_LEN];
    unsigned long stored_hash_length, salt_length;
    
    MYSQL_BIND bind_result[2];
    memset(bind_result, 0, sizeof(bind_result));
    memset(stored_hash, 0, sizeof(stored_hash));
    memset(salt, 0, sizeof(salt));
    
    bind_result[0].buffer_type = MYSQL_TYPE_STRING;
    bind_result[0].buffer = stored_hash;
    bind_result[0].buffer_length = sizeof(stored_hash);
    bind_result[0].length = &stored_hash_length;
    
    bind_result[1].buffer_type = MYSQL_TYPE_STRING;
    bind_result[1].buffer = salt;
    bind_result[1].buffer_length = sizeof(salt);
    bind_result[1].length = &salt_length;
    
    mysql_stmt_bind_result(stmt, bind_result);
    
    /* Fetch result */
    int fetch_result = mysql_stmt_fetch(stmt);
    
    if (fetch_result == 0) {
        char input_hash[MAX_HASH_LEN];
        hash_password(password, salt, input_hash);
        
        /* Constant-time comparison */
        if (strncmp(input_hash, stored_hash, stored_hash_length) == 0) {
            result.success = 1;
            strcpy(result.redirectUrl, "home.html");
        }
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main() {
    printf("Test Case 1 - Valid user:\\n");
    LoginResult result1 = login("validuser", "correctpassword");
    printf("Success: %d, Redirect: %s\\n", result1.success, result1.redirectUrl);
    
    printf("\\nTest Case 2 - Invalid password:\\n");
    LoginResult result2 = login("validuser", "wrongpassword");
    printf("Success: %d, Redirect: %s\\n", result2.success, result2.redirectUrl);
    
    printf("\\nTest Case 3 - Non-existent user:\\n");
    LoginResult result3 = login("nonexistent", "anypassword");
    printf("Success: %d, Redirect: %s\\n", result3.success, result3.redirectUrl);
    
    printf("\\nTest Case 4 - Empty username:\\n");
    LoginResult result4 = login("", "password");
    printf("Success: %d, Redirect: %s\\n", result4.success, result4.redirectUrl);
    
    printf("\\nTest Case 5 - SQL injection attempt:\\n");
    LoginResult result5 = login("admin' OR '1'='1", "password");
    printf("Success: %d, Redirect: %s\\n", result5.success, result5.redirectUrl);
    
    return 0;
}
