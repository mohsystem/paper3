
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define SALT_LEN 16
#define HASH_LEN 32
#define PBKDF2_ITERATIONS 210000

/* Secure credentials structure */
typedef struct {
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
} UserCredentials;

/* Global user database - in production use secure database */
static UserCredentials admin_creds;
static int db_initialized = 0;

/* Generate cryptographically secure random salt */
int generate_salt(unsigned char* salt, size_t length) {
    /* Use OpenSSL CSPRNG for secure randomness */
    return RAND_bytes(salt, (int)length) == 1;
}

/* Hash password using PBKDF2-HMAC-SHA256 */
int hash_password(const char* password, const unsigned char* salt,
                  size_t salt_len, unsigned char* output, size_t output_len) {
    /* PBKDF2 with 210000 iterations per OWASP recommendations */
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, (int)salt_len,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          (int)output_len, output) != 1) {
        return 0;
    }
    return 1;
}

/* Constant-time comparison to prevent timing attacks */
int constant_time_compare(const unsigned char* a, const unsigned char* b, size_t length) {
    volatile unsigned char result = 0;
    size_t i;
    for (i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Initialize database with admin user */
void initialize_database(void) {
    if (db_initialized) {
        return;
    }
    
    /* Generate random salt */
    if (!generate_salt(admin_creds.salt, SALT_LEN)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return;
    }
    
    /* Hash admin password */
    const char* admin_password = "admin123";
    if (!hash_password(admin_password, admin_creds.salt, SALT_LEN,
                      admin_creds.hash, HASH_LEN)) {
        fprintf(stderr, "Failed to hash password\\n");
        return;
    }
    
    db_initialized = 1;
}

/* Input validation: check if username is valid */
int is_valid_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        return 0;
    }
    
    len = strlen(username);
    
    /* Check length constraints */
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    /* Only allow alphanumeric and underscore */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

/* GET /login - Returns login form */
const char* login(void) {
    /* Return secure HTML form with input validation */
    return "<!DOCTYPE html>\\n"
           "<html>\\n"
           "<head>\\n"
           "    <title>Login</title>\\n"
           "    <meta charset=\\"UTF-8\\">\\n"
           "</head>\\n"
           "<body>\\n"
           "    <h2>Login Form</h2>\\n"
           "    <form method=\\"POST\\" action=\\"/do_login\\">\\n"
           "        <label>Username: \\n"
           "            <input type=\\"text\\" name=\\"username\\" required \\n"
           "                   maxlength=\\"50\\" pattern=\\"[a-zA-Z0-9_]+\\">\\n"
           "        </label><br>\\n"
           "        <label>Password: \\n"
           "            <input type=\\"password\\" name=\\"password\\" required \\n"
           "                   maxlength=\\"100\\">\\n"
           "        </label><br>\\n"
           "        <input type=\\"submit\\" value=\\"Login\\">\\n"
           "    </form>\\n"
           "</body>\\n"
           "</html>";
}

/* POST /do_login - Process login credentials */
const char* do_login(const char* username, const char* password) {
    unsigned char provided_hash[HASH_LEN];
    unsigned char dummy_salt[SALT_LEN];
    unsigned char dummy_hash[HASH_LEN];
    
    /* Input validation: check for NULL pointers */
    if (username == NULL || password == NULL) {
        return "error";
    }
    
    /* Input validation: check for empty strings */
    if (strlen(username) == 0 || strlen(password) == 0) {
        return "error";
    }
    
    /* Input validation: sanitize username */
    if (!is_valid_username(username)) {
        return "error";
    }
    
    /* Length validation for password to prevent DoS */
    if (strlen(password) > MAX_PASSWORD_LEN) {
        return "error";
    }
    
    /* Ensure database is initialized */
    if (!db_initialized) {
        return "error";
    }
    
    /* Check if username is admin */
    if (strcmp(username, "admin") != 0) {
        /* Perform dummy hash to prevent timing-based username enumeration */
        RAND_bytes(dummy_salt, SALT_LEN);
        hash_password(password, dummy_salt, SALT_LEN, dummy_hash, HASH_LEN);
        return "error";
    }
    
    /* Hash provided password with stored salt */
    if (!hash_password(password, admin_creds.salt, SALT_LEN,
                      provided_hash, HASH_LEN)) {
        return "error";
    }
    
    /* Use constant-time comparison to prevent timing attacks */
    if (constant_time_compare(admin_creds.hash, provided_hash, HASH_LEN)) {
        /* Valid admin credentials */
        return "redirect_admin";
    }
    
    /* Generic error message to prevent information disclosure */
    return "error";
}

int main(void) {
    const char* result;
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Initialize user database */
    initialize_database();
    
    /* Test Case 1 - Valid admin login */
    printf("Test Case 1 - Valid admin login:\\n");
    result = do_login("admin", "admin123");
    printf("Result: %s\\n", result);
    printf("Expected: redirect_admin\\n\\n");
    
    /* Test Case 2 - Invalid password */
    printf("Test Case 2 - Invalid password:\\n");
    result = do_login("admin", "wrongpass");
    printf("Result: %s\\n", result);
    printf("Expected: error\\n\\n");
    
    /* Test Case 3 - Invalid username */
    printf("Test Case 3 - Invalid username:\\n");
    result = do_login("hacker", "password");
    printf("Result: %s\\n", result);
    printf("Expected: error\\n\\n");
    
    /* Test Case 4 - SQL injection attempt */
    printf("Test Case 4 - SQL injection attempt:\\n");
    result = do_login("admin' OR '1'='1", "anything");
    printf("Result: %s\\n", result);
    printf("Expected: error\\n\\n");
    
    /* Test Case 5 - Empty credentials */
    printf("Test Case 5 - Empty credentials:\\n");
    result = do_login("", "");
    printf("Result: %s\\n", result);
    printf("Expected: error\\n\\n");
    
    /* Cleanup OpenSSL */
    EVP_cleanup();
    
    return 0;
}
