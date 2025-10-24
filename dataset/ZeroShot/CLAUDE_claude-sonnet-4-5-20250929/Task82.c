
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

void hashPasswordWithSalt(const char* password, const char* salt, char* output) {
    char combined[512];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    snprintf(combined, sizeof(combined), "%s%s", password, salt);
    
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
}

int main() {
    char hash[65];
    
    // Test case 1
    hashPasswordWithSalt("password123", "randomsalt1", hash);
    printf("Test 1: %s\\n", hash);
    
    // Test case 2
    hashPasswordWithSalt("securePass!", "salt2024", hash);
    printf("Test 2: %s\\n", hash);
    
    // Test case 3
    hashPasswordWithSalt("admin", "xyz789", hash);
    printf("Test 3: %s\\n", hash);
    
    // Test case 4
    hashPasswordWithSalt("MyP@ssw0rd", "abc123def", hash);
    printf("Test 4: %s\\n", hash);
    
    // Test case 5
    hashPasswordWithSalt("test", "salt", hash);
    printf("Test 5: %s\\n", hash);
    
    return 0;
}
