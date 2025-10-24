
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

void hashPassword(const char* password, const char* salt, char* output) {
    char combined[1024];
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
    hashPassword("password123", "randomsalt", hash);
    printf("Test 1: %s\\n", hash);
    
    // Test case 2
    hashPassword("mySecurePass", "salt456", hash);
    printf("Test 2: %s\\n", hash);
    
    // Test case 3
    hashPassword("admin", "abcdef", hash);
    printf("Test 3: %s\\n", hash);
    
    // Test case 4
    hashPassword("test", "xyz", hash);
    printf("Test 4: %s\\n", hash);
    
    // Test case 5
    hashPassword("", "emptysalt", hash);
    printf("Test 5: %s\\n", hash);
    
    return 0;
}
