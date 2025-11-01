
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <mysql/mysql.h>
#include <regex.h>

#define SALT_LENGTH 16
#define HASH_LENGTH 32
#define ITERATIONS 210000
#define MAX_USERNAME_LENGTH 255
#define MAX_PASSWORD_LENGTH 1024

/* Generate cryptographically secure random salt */
int generate_salt(unsigned char* salt, size_t length) {
    if (!salt) return 0;
    /* Use OpenSSL CSPRNG for secure random generation */
    return RAND_bytes(salt, length) == 1;
}

/* Secure password hashing using PBKDF2-HMAC-SHA256 */
int hash_password(const char* password, const unsigned char* salt, 
                  size_t salt_len, char* output, size_t output_len) {
    if (!password || !salt || !output || output_len < HASH_LENGTH) {
        return 0;
    }
    
    unsigned char hash[HASH_LENGTH];
    memset(hash, 0, HASH_LENGTH);
    
    /* Use PBKDF2 with SHA-256 and 210000 iterations */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, salt_len,
                          ITERATIONS,
                          EVP_sha256(),
                          HASH_LENGTH,
                          hash) != 1) {
        return 0;
    }
    
    /* Base64 encode (simplified - copy raw bytes) */
    memcpy(output, hash, HASH_LENGTH);
    
    /* Clear sensitive data using memset_s or explicit_bzero if available */
    #ifdef __STDC_LIB_EXT1__
        memset_s(hash, HASH_LENGTH, 0, HASH_LENGTH);
    #else
        volatile unsigned char* p = hash;
        while (HASH_LENGTH--) *p++ = 0;
    #endif
    
    return 1;
}

/* Validate input to prevent injection attacks */
int validate_input(const char* username, const char* password) {
    if (!username || !password) return 0;
    
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);
    
    /* Length validation */
    if (username_len == 0 || username_len > MAX_USERNAME_LENGTH ||
        password_len == 0 || password_len > MAX_PASSWORD_LENGTH) {
        return 0;
    }
    
    /* Username format validation using regex */
    regex_t regex;
    int result;
    
    if (regcomp(&regex, "^[a-zA-Z0-9_]{3,255}$", REG_EXTENDED | REG_NOSUB) != 0) {
        return 0;
    }
    
    result = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

/* Constant-time comparison to prevent timing attacks */
int secure_compare(const char* a, const char* b, size_t len) {
    if (!a || !b) return 0;
    return CRYPTO_memcmp(a, b, len) == 0;
}

/* Secure login function with parameterized queries */
char* login(const char* username, const char* password) {
    static char redirect[256];
    
    /* Input validation */
    if (!validate_input(username, password)) {
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=invalid");
        return redirect;
    }
    
    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[2];
    
    /* Initialize connection */
    conn = mysql_init(NULL);
    if (!conn) {
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Enable TLS */
    mysql_ssl_set(conn, NULL, NULL, NULL, NULL, NULL);
    unsigned int ssl_mode = SSL_MODE_REQUIRED;
    mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);
    
    /* Get credentials from environment */
    const char* host = getenv("DB_HOST");
    const char* user = getenv("DB_USER");
    const char* db_pass = getenv("DB_PASSWORD");
    const char* database = getenv("DB_NAME");
    
    /* Connect to database */
    if (!mysql_real_connect(conn, host, user, db_pass, database, 0, NULL, 0)) {
        mysql_close(conn);
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Prepare parameterized statement */
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ? AND account_locked = FALSE LIMIT 1";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        if (stmt) mysql_stmt_close(stmt);
        mysql_close(conn);
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Bind input parameter */
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(username);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = username_length;
    bind[0].length = &username_length;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Execute query */
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Bind result */
    char stored_hash[256];
    char salt_str[256];
    unsigned long hash_length = 0, salt_length = 0;
    my_bool is_null[2] = {0, 0};
    
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = stored_hash;
    result[0].buffer_length = sizeof(stored_hash);
    result[0].length = &hash_length;
    result[0].is_null = &is_null[0];
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = salt_str;
    result[1].buffer_length = sizeof(salt_str);
    result[1].length = &salt_length;
    result[1].is_null = &is_null[1];
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
        return redirect;
    }
    
    /* Fetch result */
    int fetch_result = mysql_stmt_fetch(stmt);
    
    if (fetch_result == 0 && hash_length > 0 && salt_length > 0) {
        stored_hash[hash_length] = '\\0';
        salt_str[salt_length] = '\\0';
        
        /* Hash provided password */
        char computed_hash[HASH_LENGTH];
        if (!hash_password(password, (unsigned char*)salt_str, 
                          salt_length, computed_hash, HASH_LENGTH)) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            snprintf(redirect, sizeof(redirect), "redirect:/login?error=system");
            return redirect;
        }
        
        /* Constant-time comparison */
        if (secure_compare(computed_hash, stored_hash, HASH_LENGTH)) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            snprintf(redirect, sizeof(redirect), "redirect:/dashboard");
            return redirect;
        }
    }
    
    /* Failed authentication or user not found */
    mysql_stmt_close(stmt);
    mysql_close(conn);
    snprintf(redirect, sizeof(redirect), "redirect:/login?error=invalid");
    return redirect;
}

int main(void) {
    /* Test cases */
    printf("Test 1: %s\\n", login("validuser", "ValidPass123!"));
    printf("Test 2: %s\\n", login("validuser", "wrongpassword"));
    printf("Test 3: %s\\n", login("nonexistent", "password"));
    printf("Test 4: %s\\n", login("admin' OR '1'='1", "password"));
    printf("Test 5: %s\\n", login("", ""));
    
    return 0;
}
