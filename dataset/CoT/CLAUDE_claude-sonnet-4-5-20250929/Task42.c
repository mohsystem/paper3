
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <time.h>

#define SALT_LENGTH 16
#define MAX_LOGIN_ATTEMPTS 3
#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define BASE64_LENGTH(len) (((len + 2) / 3) * 4)

typedef struct {
    char passwordHash[BASE64_LENGTH(SHA256_DIGEST_LENGTH) + 1];
    char salt[BASE64_LENGTH(SALT_LENGTH) + 1];
} UserCredentials;

typedef struct {
    char username[MAX_USERNAME_LEN];
    UserCredentials credentials;
    int loginAttempts;
    int active;
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
} Task42;

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, size_t length, char* output) {
    size_t i, j;
    for (i = 0, j = 0; i < length; i += 3, j += 4) {
        unsigned int triple = (i < length ? input[i] : 0) << 16;
        triple |= (i + 1 < length ? input[i + 1] : 0) << 8;
        triple |= (i + 2 < length ? input[i + 2] : 0);
        
        output[j] = base64_table[(triple >> 18) & 0x3F];
        output[j + 1] = base64_table[(triple >> 12) & 0x3F];
        output[j + 2] = (i + 1 < length) ? base64_table[(triple >> 6) & 0x3F] : '=';
        output[j + 3] = (i + 2 < length) ? base64_table[triple & 0x3F] : '=';
    }
    output[j] = '\\0';
}

int base64_decode_char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

size_t base64_decode(const char* input, unsigned char* output) {
    size_t i, j;
    int val1, val2, val3, val4;
    size_t len = strlen(input);
    
    for (i = 0, j = 0; i < len; i += 4) {
        val1 = base64_decode_char(input[i]);
        val2 = base64_decode_char(input[i + 1]);
        val3 = (input[i + 2] == '=') ? 0 : base64_decode_char(input[i + 2]);
        val4 = (input[i + 3] == '=') ? 0 : base64_decode_char(input[i + 3]);
        
        output[j++] = (val1 << 2) | (val2 >> 4);
        if (input[i + 2] != '=') output[j++] = (val2 << 4) | (val3 >> 2);
        if (input[i + 3] != '=') output[j++] = (val3 << 6) | val4;
    }
    
    return j;
}

void generate_salt(char* salt_output) {
    unsigned char salt[SALT_LENGTH];
    srand(time(NULL));
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt[i] = rand() % 256;
    }
    base64_encode(salt, SALT_LENGTH, salt_output);
}

void hash_password(const char* password, const char* salt_b64, char* hash_output) {
    unsigned char salt[SALT_LENGTH];
    size_t salt_len = base64_decode(salt_b64, salt);
    
    size_t combined_len = salt_len + strlen(password);
    unsigned char* combined = (unsigned char*)malloc(combined_len);
    memcpy(combined, salt, salt_len);
    memcpy(combined + salt_len, password, strlen(password));
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(combined, combined_len, hash);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, hash_output);
    free(combined);
}

void init_auth_system(Task42* system) {
    system->userCount = 0;
    memset(system->users, 0, sizeof(system->users));
}

int find_user(Task42* system, const char* username) {
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int register_user(Task42* system, const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) < 8) {
        return 0;
    }
    
    if (system->userCount >= MAX_USERS) {
        return 0;
    }
    
    if (find_user(system, username) != -1) {
        return 0;
    }
    
    User* new_user = &system->users[system->userCount];
    strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
    new_user->username[MAX_USERNAME_LEN - 1] = '\\0';
    
    generate_salt(new_user->credentials.salt);
    hash_password(password, new_user->credentials.salt, new_user->credentials.passwordHash);
    
    new_user->loginAttempts = 0;
    new_user->active = 1;
    system->userCount++;
    
    return 1;
}

int authenticate_user(Task42* system, const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    int user_idx = find_user(system, username);
    if (user_idx == -1) {
        return 0;
    }
    
    User* user = &system->users[user_idx];
    
    if (user->loginAttempts >= MAX_LOGIN_ATTEMPTS) {
        return 0;
    }
    
    char password_hash[BASE64_LENGTH(SHA256_DIGEST_LENGTH) + 1];
    hash_password(password, user->credentials.salt, password_hash);
    
    if (strcmp(password_hash, user->credentials.passwordHash) == 0) {
        user->loginAttempts = 0;
        return 1;
    } else {
        user->loginAttempts++;
        return 0;
    }
}

int change_password(Task42* system, const char* username, const char* old_password, const char* new_password) {
    if (!authenticate_user(system, username, old_password)) {
        return 0;
    }
    
    if (new_password == NULL || strlen(new_password) < 8) {
        return 0;
    }
    
    int user_idx = find_user(system, username);
    User* user = &system->users[user_idx];
    
    generate_salt(user->credentials.salt);
    hash_password(new_password, user->credentials.salt, user->credentials.passwordHash);
    
    return 1;
}

void reset_login_attempts(Task42* system, const char* username) {
    int user_idx = find_user(system, username);
    if (user_idx != -1) {
        system->users[user_idx].loginAttempts = 0;
    }
}

int main() {
    Task42 authSystem;
    init_auth_system(&authSystem);
    
    printf("Test Case 1: User Registration\\n");
    int result1 = register_user(&authSystem, "alice", "SecurePass123");
    printf("Register alice: %s\\n", result1 ? "SUCCESS" : "FAILED");
    
    printf("\\nTest Case 2: Successful Authentication\\n");
    int result2 = authenticate_user(&authSystem, "alice", "SecurePass123");
    printf("Authenticate alice: %s\\n", result2 ? "SUCCESS" : "FAILED");
    
    printf("\\nTest Case 3: Failed Authentication (wrong password)\\n");
    int result3 = authenticate_user(&authSystem, "alice", "WrongPass123");
    printf("Authenticate with wrong password: %s\\n", result3 ? "SUCCESS" : "FAILED");
    
    printf("\\nTest Case 4: Password Change\\n");
    int result4 = change_password(&authSystem, "alice", "SecurePass123", "NewSecure456");
    printf("Change password: %s\\n", result4 ? "SUCCESS" : "FAILED");
    int result4b = authenticate_user(&authSystem, "alice", "NewSecure456");
    printf("Authenticate with new password: %s\\n", result4b ? "SUCCESS" : "FAILED");
    
    printf("\\nTest Case 5: Account Lockout after multiple failed attempts\\n");
    register_user(&authSystem, "bob", "BobPass123");
    authenticate_user(&authSystem, "bob", "wrong1");
    authenticate_user(&authSystem, "bob", "wrong2");
    authenticate_user(&authSystem, "bob", "wrong3");
    int result5 = authenticate_user(&authSystem, "bob", "BobPass123");
    printf("Authenticate after 3 failed attempts: %s\\n", result5 ? "SUCCESS" : "FAILED (LOCKED)");
    
    return 0;
}
