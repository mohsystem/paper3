
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t length) {
    size_t output_length = 4 * ((length + 2) / 3);
    char* encoded = malloc(output_length + 1);
    if (!encoded) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? data[i++] : 0;
        uint32_t octet_b = i < length ? data[i++] : 0;
        uint32_t octet_c = i < length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded[j++] = base64_chars[triple & 0x3F];
    }
    
    for (i = 0; i < (3 - length % 3) % 3; i++)
        encoded[output_length - 1 - i] = '=';
    
    encoded[output_length] = '\\0';
    return encoded;
}

unsigned char* base64_decode(const char* input, size_t* output_length) {
    size_t input_length = strlen(input);
    *output_length = input_length / 4 * 3;
    if (input[input_length - 1] == '=') (*output_length)--;
    if (input[input_length - 2] == '=') (*output_length)--;
    
    unsigned char* decoded = malloc(*output_length);
    if (!decoded) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t a = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t b = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t c = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t d = input[i] == '=' ? 0 & i++ : strchr(base64_chars, input[i++]) - base64_chars;
        uint32_t triple = (a << 18) + (b << 12) + (c << 6) + d;
        
        if (j < *output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded[j++] = triple & 0xFF;
    }
    
    return decoded;
}

char* generate_salt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    return base64_encode(salt, 16);
}

char* hash_password(const char* password, const char* salt) {
    size_t salt_length;
    unsigned char* decoded_salt = base64_decode(salt, &salt_length);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, decoded_salt, salt_length);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    free(decoded_salt);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

char* register_user(const char* username, const char* password) {
    char* salt = generate_salt();
    char* hashed_password = hash_password(password, salt);
    
    size_t stored_length = strlen(salt) + strlen(hashed_password) + 2;
    char* stored_value = malloc(stored_length);
    snprintf(stored_value, stored_length, "%s:%s", salt, hashed_password);
    
    free(salt);
    free(hashed_password);
    return stored_value;
}

int main() {
    // Test case 1
    char* result1 = register_user("user1", "password123");
    printf("Test 1 - User: user1, Stored: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = register_user("alice", "securePass456");
    printf("Test 2 - User: alice, Stored: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = register_user("bob", "myP@ssw0rd");
    printf("Test 3 - User: bob, Stored: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = register_user("admin", "Admin#2024");
    printf("Test 4 - User: admin, Stored: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = register_user("testuser", "Test!ng123");
    printf("Test 5 - User: testuser, Stored: %s\\n", result5);
    free(result5);
    
    return 0;
}
