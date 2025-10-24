#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

// --- Security Constants ---
#define SALT_LEN 16
#define TOKEN_LEN 32
#define TOKEN_VALIDITY_MINUTES 15
#define MIN_PASSWORD_LENGTH 8
#define MAX_USERNAME_LEN 50
#define HASH_OUTPUT_LEN 65 // 64 hex chars + null terminator
#define TOKEN_OUTPUT_LEN (TOKEN_LEN * 2 + 1)
#define SALT_OUTPUT_LEN (SALT_LEN * 2 + 1)
#define MAX_USERS 10

// --- Embedded SHA256 Implementation ---
// This is a minimal, public-domain SHA256 implementation for demonstration.
// **WARNING**: For production code, use a proper, well-vetted cryptography library
// like OpenSSL or libsodium to implement a key-stretching function like Argon2, scrypt, or PBKDF2.
// A simple salted SHA256 is NOT sufficient for storing passwords securely.
typedef struct {
    uint8_t buf[64];
    uint32_t state[8];
    uint32_t bitlen[2];
} SHA256_CTX;

const uint32_t SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define F1(x, y, z) (((x) & (y)) | ((z) & ((x) | (y))))
#define F2(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define S3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define S4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i)
        m[i] = S4(m[i - 2]) + m[i - 7] + S3(m[i - 15]) + m[i - 16];
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0; i < 64; ++i) {
        t1 = h + S1(e) + F1(e, f, g) + SHA256_K[i] + m[i];
        t2 = F2(a) + F1(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a; ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
    ctx->bitlen[0] = 0; ctx->bitlen[1] = 0;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->buf[ctx->bitlen[0] / 8 % 64] = data[i];
        ctx->bitlen[0] += 8;
        if (ctx->bitlen[0] % 512 == 0) {
            sha256_transform(ctx, ctx->buf);
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->bitlen[0] / 8 % 64;
    ctx->buf[i++] = 0x80;
    while (i < 56) ctx->buf[i++] = 0x00;
    uint64_t len_bits = ctx->bitlen[0];
    for(i=0; i<8; ++i) ctx->buf[56+i] = (len_bits >> (56-i*8)) & 0xFF;
    sha256_transform(ctx, ctx->buf);
    for (i = 0; i < 8; ++i) {
        hash[i*4+0] = (ctx->state[i] >> 24) & 0xff;
        hash[i*4+1] = (ctx->state[i] >> 16) & 0xff;
        hash[i*4+2] = (ctx->state[i] >> 8) & 0xff;
        hash[i*4+3] = ctx->state[i] & 0xff;
    }
}
// --- End SHA256 Implementation ---

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[HASH_OUTPUT_LEN];
    char salt[SALT_OUTPUT_LEN];
    char resetToken[TOKEN_OUTPUT_LEN];
    time_t tokenExpiry;
} User;

// --- In-memory user store (for simulation) ---
static User userDatabase[MAX_USERS];
static int user_count = 0;

// --- Helper Functions ---
void bytes_to_hex(const uint8_t* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
}

void generate_random_bytes(uint8_t* buf, size_t len) {
    // NOTE: This uses /dev/urandom and is not portable to Windows.
    // Windows would use CryptGenRandom().
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(buf, 1, len, f);
        fclose(f);
    } else {
        // Fallback for systems without /dev/urandom (less secure)
        for (size_t i = 0; i < len; i++) {
            buf[i] = rand();
        }
    }
}

void hash_password(const char* password, const char* salt, char* output) {
    // **SECURITY WARNING**: This is a simplified hash for demonstration.
    // DO NOT USE this in production. Use PBKDF2, scrypt, or Argon2.
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", password, salt);
    
    uint8_t hash[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t*)combined, strlen(combined));
    sha256_final(&ctx, hash);
    bytes_to_hex(hash, 32, output);
}

User* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

bool validate_password_policy(const char* password) {
    bool has_digit = false, has_lower = false, has_upper = false, has_special = false;
    const char* specials = "@#$%^&+=!";
    if (strlen(password) < MIN_PASSWORD_LENGTH) return false;

    for (const char* p = password; *p; ++p) {
        if (*p >= '0' && *p <= '9') has_digit = true;
        else if (*p >= 'a' && *p <= 'z') has_lower = true;
        else if (*p >= 'A' && *p <= 'Z') has_upper = true;
        else if (strchr(specials, *p)) has_special = true;
    }
    return has_digit && has_lower && has_upper && has_special;
}

// --- Core Logic ---
bool requestPasswordReset(const char* username, char* out_token) {
    User* user = find_user(username);
    if (!user) {
        return false;
    }
    
    uint8_t token_bytes[TOKEN_LEN];
    generate_random_bytes(token_bytes, TOKEN_LEN);
    bytes_to_hex(token_bytes, TOKEN_LEN, user->resetToken);
    
    user->tokenExpiry = time(NULL) + TOKEN_VALIDITY_MINUTES * 60;
    strcpy(out_token, user->resetToken);
    return true;
}

bool resetPassword(const char* username, const char* token, const char* newPassword) {
    User* user = find_user(username);
    if (!user || !token || !newPassword) return false;

    // 1. Validate token
    if (user->resetToken[0] == '\0' || strcmp(user->resetToken, token) != 0) {
        return false;
    }
    
    // 2. Check expiration
    if (time(NULL) > user->tokenExpiry) {
        user->resetToken[0] = '\0'; // Expired, invalidate
        return false;
    }
    
    // 3. Invalidate token immediately
    user->resetToken[0] = '\0';

    // 4. Validate password policy
    if (!validate_password_policy(newPassword)) {
        return false;
    }

    // 5. Update password
    uint8_t salt_bytes[SALT_LEN];
    generate_random_bytes(salt_bytes, SALT_LEN);
    bytes_to_hex(salt_bytes, SALT_LEN, user->salt);
    hash_password(newPassword, user->salt, user->hashedPassword);
    
    return true;
}

bool checkPassword(const char* username, const char* password) {
    User* user = find_user(username);
    if (!user) return false;
    char attempt_hash[HASH_OUTPUT_LEN];
    hash_password(password, user->salt, attempt_hash);
    return strcmp(user->hashedPassword, attempt_hash) == 0;
}

int main() {
    srand(time(NULL)); // Seed for fallback random generation
    
    // --- Setup: Create some users ---
    const char* initialPassAlice = "AlicePass123!";
    uint8_t salt_bytes_alice[SALT_LEN];
    generate_random_bytes(salt_bytes_alice, SALT_LEN);
    strcpy(userDatabase[user_count].username, "alice");
    bytes_to_hex(salt_bytes_alice, SALT_LEN, userDatabase[user_count].salt);
    hash_password(initialPassAlice, userDatabase[user_count].salt, userDatabase[user_count].hashedPassword);
    user_count++;
    
    const char* initialPassBob = "BobSecure@2023";
    uint8_t salt_bytes_bob[SALT_LEN];
    generate_random_bytes(salt_bytes_bob, SALT_LEN);
    strcpy(userDatabase[user_count].username, "bob");
    bytes_to_hex(salt_bytes_bob, SALT_LEN, userDatabase[user_count].salt);
    hash_password(initialPassBob, userDatabase[user_count].salt, userDatabase[user_count].hashedPassword);
    user_count++;
    
    printf("--- Running Password Reset Test Cases ---\n");
    char token_buffer[TOKEN_OUTPUT_LEN];

    // --- Test Case 1: Successful Reset ---
    printf("\n[Test Case 1: Successful Reset]\n");
    bool token_ok1 = requestPasswordReset("alice", token_buffer);
    printf("Alice requested reset. Token received: %s\n", token_ok1 ? "true" : "false");
    const char* newPassword1 = "NewSecurePass!456";
    bool success1 = resetPassword("alice", token_buffer, newPassword1);
    printf("Password reset attempt with valid token: %s\n", success1 ? "SUCCESS" : "FAIL");
    printf("Alice can log in with new password: %s\n", checkPassword("alice", newPassword1) ? "true" : "false");
    printf("Alice cannot log in with old password: %s\n", !checkPassword("alice", initialPassAlice) ? "true" : "false");

    // --- Test Case 2: Invalid Token ---
    printf("\n[Test Case 2: Invalid Token]\n");
    bool token_ok2 = requestPasswordReset("bob", token_buffer);
    printf("Bob requested reset. Token received: %s\n", token_ok2 ? "true" : "false");
    bool success2 = resetPassword("bob", "invalid-token-string", "SomePassword1!");
    printf("Password reset attempt with invalid token: %s\n", success2 ? "SUCCESS" : "FAIL");
    printf("Bob can still log in with old password: %s\n", checkPassword("bob", initialPassBob) ? "true" : "false");
    
    // --- Test Case 3: Expired Token ---
    printf("\n[Test Case 3: Expired Token]\n");
    bool token_ok3 = requestPasswordReset("bob", token_buffer);
    find_user("bob")->tokenExpiry = time(NULL) - 1; // Manually expire
    printf("Bob requested reset. Token is now expired.\n");
    bool success3 = resetPassword("bob", token_buffer, "AnotherPass!789");
    printf("Password reset attempt with expired token: %s\n", success3 ? "SUCCESS" : "FAIL");
    printf("Bob can still log in with old password: %s\n", checkPassword("bob", initialPassBob) ? "true" : "false");

    // --- Test Case 4: Weak Password ---
    printf("\n[Test Case 4: Weak Password]\n");
    bool token_ok4 = requestPasswordReset("alice", token_buffer);
    printf("Alice requested reset again.\n");
    bool success4 = resetPassword("alice", token_buffer, "weak");
    printf("Password reset attempt with weak password: %s\n", success4 ? "SUCCESS" : "FAIL");
    printf("Alice can still log in with her last valid password: %s\n", checkPassword("alice", newPassword1) ? "true" : "false");

    // --- Test Case 5: Non-Existent User ---
    printf("\n[Test Case 5: Non-Existent User]\n");
    bool token_ok5 = requestPasswordReset("dave", token_buffer);
    printf("Request reset for non-existent user 'dave'. Token received: %s\n", token_ok5 ? "true" : "false");
    bool success5 = resetPassword("dave", "any-token", "anyPassword1!");
    printf("Password reset attempt for non-existent user: %s\n", success5 ? "SUCCESS" : "FAIL");
    
    return 0;
}