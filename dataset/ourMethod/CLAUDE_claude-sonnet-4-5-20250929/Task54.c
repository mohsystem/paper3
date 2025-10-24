
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* Use OpenSSL for cryptographic operations */
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

/* Constants for security */
#define OTP_LENGTH 6
#define SALT_SIZE 16
#define PBKDF2_ITERATIONS 210000
#define HASH_SIZE 32
#define OTP_VALIDITY_SECONDS 300
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128
#define MAX_INPUT_LEN 256

/* Structure to hold user credentials securely */
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char password_hash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
} UserCredentials;

/* Structure to hold OTP data */
typedef struct {
    char otp[OTP_LENGTH + 1];
    time_t created_at;
} OTPData;

/* Securely clear memory */
void secure_clear(void* ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        OPENSSL_cleanse(ptr, size);
    }
}

/* Initialize user credentials structure */
void init_user_credentials(UserCredentials* creds) {
    if (creds == NULL) return;
    
    memset(creds->username, 0, sizeof(creds->username));
    memset(creds->password_hash, 0, sizeof(creds->password_hash));
    memset(creds->salt, 0, sizeof(creds->salt));
}

/* Cleanup user credentials securely */
void cleanup_user_credentials(UserCredentials* creds) {
    if (creds == NULL) return;
    
    secure_clear(creds->password_hash, HASH_SIZE);
    secure_clear(creds->salt, SALT_SIZE);
}

/* Generate cryptographically secure random OTP */
/* Returns 1 on success, 0 on failure */
int generate_secure_otp(char* otp_out) {
    if (otp_out == NULL) {
        fprintf(stderr, "Error: NULL output buffer\\n");
        return 0;
    }
    
    unsigned char random_bytes[OTP_LENGTH];
    memset(random_bytes, 0, OTP_LENGTH);
    
    /* Use OpenSSL's cryptographically secure RNG */\n    if (RAND_bytes(random_bytes, OTP_LENGTH) != 1) {\n        fprintf(stderr, "Error: Failed to generate secure random bytes\\n");\n        secure_clear(random_bytes, OTP_LENGTH);\n        return 0;\n    }\n    \n    /* Convert to numeric digits (0-9) */\n    for (size_t i = 0; i < OTP_LENGTH; i++) {\n        /* Check buffer bounds */\n        if (i >= OTP_LENGTH) {\n            secure_clear(random_bytes, OTP_LENGTH);\n            return 0;\n        }\n        otp_out[i] = '0' + (random_bytes[i] % 10);\n    }\n    otp_out[OTP_LENGTH] = '\\0';\n    \n    /* Securely clear random bytes */\n    secure_clear(random_bytes, OTP_LENGTH);\n    \n    return 1;\n}\n\n/* Generate unique salt for each user */\nint generate_salt(unsigned char* salt) {\n    if (salt == NULL) {\n        fprintf(stderr, "Error: NULL salt buffer\\n");\n        return 0;\n    }\n    \n    /* Use OpenSSL's cryptographically secure RNG */
    return RAND_bytes(salt, SALT_SIZE) == 1;
}

/* Derive password hash using PBKDF2-HMAC-SHA256 */
int derive_password_hash(const char* password, const unsigned char* salt,
                        unsigned char* out_hash) {
    if (password == NULL || salt == NULL || out_hash == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    size_t password_len = strlen(password);
    
    /* Validate password length to prevent excessive memory usage */
    if (password_len == 0 || password_len > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Error: Invalid password length\\n");
        return 0;
    }
    
    /* Use PBKDF2-HMAC-SHA256 with 210000 iterations */
    int result = PKCS5_PBKDF2_HMAC(
        password,
        (int)password_len,
        salt,
        SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_SIZE,
        out_hash
    );
    
    return result == 1;
}

/* Register a new user with secure password hashing */
int register_user(const char* username, const char* password,
                 UserCredentials* creds) {
    if (username == NULL || password == NULL || creds == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);
    
    /* Validate inputs - treat all inputs as untrusted */
    if (username_len == 0 || username_len > MAX_USERNAME_LEN) {
        fprintf(stderr, "Error: Invalid username length\\n");
        return 0;
    }
    
    if (password_len < 8 || password_len > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Error: Password must be 8-128 characters\\n");
        return 0;
    }
    
    /* Initialize credentials structure */
    init_user_credentials(creds);
    
    /* Generate unique salt */
    if (!generate_salt(creds->salt)) {
        fprintf(stderr, "Error: Failed to generate salt\\n");
        return 0;
    }
    
    /* Derive password hash with salt */
    if (!derive_password_hash(password, creds->salt, creds->password_hash)) {
        fprintf(stderr, "Error: Failed to hash password\\n");
        secure_clear(creds->salt, SALT_SIZE);
        return 0;
    }
    
    /* Copy username with bounds check */
    strncpy(creds->username, username, MAX_USERNAME_LEN);
    creds->username[MAX_USERNAME_LEN] = '\\0';
    
    return 1;
}

/* Verify password using constant-time comparison */
int verify_password(const char* password, const UserCredentials* creds) {
    if (password == NULL || creds == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    if (strlen(password) == 0) {
        return 0;
    }
    
    unsigned char computed_hash[HASH_SIZE];
    memset(computed_hash, 0, HASH_SIZE);
    
    /* Derive hash from provided password */
    if (!derive_password_hash(password, creds->salt, computed_hash)) {
        secure_clear(computed_hash, HASH_SIZE);
        return 0;
    }
    
    /* Use constant-time comparison to prevent timing attacks */
    int result = CRYPTO_memcmp(computed_hash, creds->password_hash, HASH_SIZE);
    
    /* Securely clear computed hash */
    secure_clear(computed_hash, HASH_SIZE);
    
    return result == 0;
}

/* Verify OTP with timing check */
int verify_otp(const char* provided_otp, const OTPData* otp_data) {
    if (provided_otp == NULL || otp_data == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    /* Check if OTP has expired */
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        fprintf(stderr, "Error: Failed to get current time\\n");
        return 0;
    }
    
    double elapsed = difftime(now, otp_data->created_at);
    
    if (elapsed > OTP_VALIDITY_SECONDS || elapsed < 0) {
        fprintf(stderr, "Error: OTP expired\\n");
        return 0;
    }
    
    size_t provided_len = strlen(provided_otp);
    
    /* Validate input format */
    if (provided_len != OTP_LENGTH) {
        return 0;
    }
    
    /* Check all characters are digits */
    for (size_t i = 0; i < provided_len; i++) {
        if (provided_otp[i] < '0' || provided_otp[i] > '9') {
            return 0;
        }
    }
    
    /* Use constant-time comparison */
    int result = CRYPTO_memcmp(provided_otp, otp_data->otp, OTP_LENGTH);
    return result == 0;
}

/* Two-factor authentication login */
int two_factor_login(const char* username, const char* password,
                    const char* otp_input, const UserCredentials* creds,
                    const OTPData* otp_data) {
    if (username == NULL || password == NULL || otp_input == NULL ||
        creds == NULL || otp_data == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    /* Step 1: Verify username */
    if (strcmp(username, creds->username) != 0) {
        fprintf(stderr, "Error: Invalid username\\n");
        return 0;
    }
    
    /* Step 2: Verify password */
    if (!verify_password(password, creds)) {
        fprintf(stderr, "Error: Invalid password\\n");
        return 0;
    }
    
    /* Step 3: Verify OTP */
    if (!verify_otp(otp_input, otp_data)) {
        fprintf(stderr, "Error: Invalid or expired OTP\\n");
        return 0;
    }
    
    return 1;
}

/* Safely read input with length validation */
int read_input(const char* prompt, char* buffer, size_t max_len) {
    if (prompt == NULL || buffer == NULL || max_len == 0) {
        return 0;
    }
    
    printf("%s", prompt);
    
    if (fgets(buffer, (int)max_len, stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return 0;
    }
    
    /* Remove newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
    }
    
    return 1;
}

int main(void) {
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    printf("=== Two-Factor Authentication System ===\\n");
    
    /* Test Case 1: Successful registration and login */
    printf("\\nTest Case 1: Successful 2FA flow\\n");
    {
        UserCredentials user;
        init_user_credentials(&user);
        
        if (!register_user("alice", "SecurePass123!", &user)) {
            fprintf(stderr, "Registration failed\\n");
            cleanup_user_credentials(&user);
            return 1;
        }
        
        OTPData otp_data;
        memset(&otp_data, 0, sizeof(OTPData));
        otp_data.created_at = time(NULL);
        
        if (!generate_secure_otp(otp_data.otp)) {
            fprintf(stderr, "OTP generation failed\\n");
            cleanup_user_credentials(&user);
            return 1;
        }
        
        printf("User registered: alice\\n");
        printf("Generated OTP: %s (valid for %d seconds)\\n",
               otp_data.otp, OTP_VALIDITY_SECONDS);
        
        if (two_factor_login("alice", "SecurePass123!", otp_data.otp,
                           &user, &otp_data)) {
            printf("✓ Login successful!\\n");
        } else {
            printf("✗ Login failed\\n");
        }
        
        cleanup_user_credentials(&user);
    }
    
    /* Test Case 2: Wrong password */
    printf("\\nTest Case 2: Wrong password\\n");
    {
        UserCredentials user;
        init_user_credentials(&user);
        
        register_user("bob", "MyPassword456", &user);
        
        OTPData otp_data;
        memset(&otp_data, 0, sizeof(OTPData));
        otp_data.created_at = time(NULL);
        generate_secure_otp(otp_data.otp);
        
        printf("Generated OTP: %s\\n", otp_data.otp);
        
        if (two_factor_login("bob", "WrongPassword", otp_data.otp,
                           &user, &otp_data)) {
            printf("✓ Login successful!\\n");
        } else {
            printf("✗ Login failed (expected)\\n");
        }
        
        cleanup_user_credentials(&user);
    }
    
    /* Test Case 3: Wrong OTP */
    printf("\\nTest Case 3: Wrong OTP\\n");
    {
        UserCredentials user;
        init_user_credentials(&user);
        
        register_user("charlie", "StrongPass789", &user);
        
        OTPData otp_data;
        memset(&otp_data, 0, sizeof(OTPData));
        otp_data.created_at = time(NULL);
        generate_secure_otp(otp_data.otp);
        
        printf("Generated OTP: %s\\n", otp_data.otp);
        
        if (two_factor_login("charlie", "StrongPass789", "000000",
                           &user, &otp_data)) {
            printf("✓ Login successful!\\n");
        } else {
            printf("✗ Login failed (expected)\\n");
        }
        
        cleanup_user_credentials(&user);
    }
    
    /* Test Case 4: Expired OTP */
    printf("\\nTest Case 4: Expired OTP\\n");
    {
        UserCredentials user;
        init_user_credentials(&user);
        
        register_user("david", "Password2024", &user);
        
        OTPData otp_data;
        memset(&otp_data, 0, sizeof(OTPData));
        generate_secure_otp(otp_data.otp);
        /* Simulate expired OTP by setting old timestamp */
        otp_data.created_at = time(NULL) - (OTP_VALIDITY_SECONDS + 1);
        
        printf("Generated OTP: %s (expired)\\n", otp_data.otp);
        
        if (two_factor_login("david", "Password2024", otp_data.otp,
                           &user, &otp_data)) {
            printf("✓ Login successful!\\n");
        } else {
            printf("✗ Login failed (expected)\\n");
        }
        
        cleanup_user_credentials(&user);
    }
    
    /* Test Case 5: Multiple OTP generations */
    printf("\\nTest Case 5: Multiple OTP generations\\n");
    {
        UserCredentials user;
        init_user_credentials(&user);
        
        register_user("eve", "ComplexPass!23", &user);
        
        printf("Generating 3 unique OTPs:\\n");
        for (int i = 0; i < 3; i++) {
            OTPData otp_data;
            memset(&otp_data, 0, sizeof(OTPData));
            otp_data.created_at = time(NULL);
            generate_secure_otp(otp_data.otp);
            
            printf("  OTP %d: %s\\n", i + 1, otp_data.otp);
            
            /* Verify each OTP works */
            if (two_factor_login("eve", "ComplexPass!23", otp_data.otp,
                               &user, &otp_data)) {
                printf("  ✓ OTP %d verified\\n", i + 1);
            }
        }
        
        cleanup_user_credentials(&user);
    }
    
    /* Cleanup OpenSSL */
    EVP_cleanup();
    
    return 0;
}
