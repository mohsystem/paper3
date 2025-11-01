
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

char* base64_encode(const unsigned char* buffer, size_t length) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    size_t output_length = 4 * ((length + 2) / 3);
    char* result = (char*)malloc(output_length + 1);
    
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(buffer++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                result[j++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(size_t k = i; k < 3; k++)
            char_array_3[k] = '\\0';
            
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (size_t k = 0; k < i + 1; k++)
            result[j++] = base64_chars[char_array_4[k]];
            
        while(i++ < 3)
            result[j++] = '=';
    }
    
    result[j] = '\\0';
    return result;
}

char* generateSalt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    return base64_encode(salt, 16);
}

char* hashPassword(const char* password, const char* salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

char* registerUser(const char* username, const char* password) {
    char* salt = generateSalt();
    char* hashedPassword = hashPassword(password, salt);
    
    size_t len = strlen(salt) + strlen(hashedPassword) + 2;
    char* result = (char*)malloc(len);
    sprintf(result, "%s:%s", salt, hashedPassword);
    
    free(salt);
    free(hashedPassword);
    
    return result;
}

int main() {
    // Test case 1
    char* result1 = registerUser("user1", "password123");
    printf("User1 registration: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = registerUser("alice", "securePass!@#");
    printf("Alice registration: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = registerUser("bob", "mySecret");
    printf("Bob registration: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = registerUser("admin", "Admin@2024");
    printf("Admin registration: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = registerUser("test_user", "Test1234");
    printf("Test_user registration: %s\\n", result5);
    free(result5);
    
    return 0;
}
