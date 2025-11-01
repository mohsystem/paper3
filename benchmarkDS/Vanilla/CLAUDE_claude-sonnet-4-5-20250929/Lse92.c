
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

void hashPassword(const char* password, char* outputBuffer) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\\0';
}

void registerUser(const char* username, const char* password) {
    char hashedPassword[65];
    hashPassword(password, hashedPassword);
    printf("Username: %s\\n", username);
    printf("Hashed Password: %s\\n", hashedPassword);
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    registerUser("john_doe", "password123");
    printf("\\n");
    
    printf("Test Case 2:\\n");
    registerUser("alice", "securePass!@#");
    printf("\\n");
    
    printf("Test Case 3:\\n");
    registerUser("bob_smith", "myP@ssw0rd");
    printf("\\n");
    
    printf("Test Case 4:\\n");
    registerUser("user123", "12345678");
    printf("\\n");
    
    printf("Test Case 5:\\n");
    registerUser("admin", "adminPass2024");
    printf("\\n");
    
    return 0;
}
