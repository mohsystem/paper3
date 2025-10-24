
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

const unsigned char STATIC_IV[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const int BLOCK_SIZE = 16;

unsigned char* padData(const unsigned char* data, int dataLen, int blockSize, int* paddedLen) {
    int paddingLength = blockSize - (dataLen % blockSize);
    if (paddingLength == 0) {
        paddingLength = blockSize;
    }
    
    *paddedLen = dataLen + paddingLength;
    unsigned char* paddedData = (unsigned char*)malloc(*paddedLen);
    
    memcpy(paddedData, data, dataLen);
    
    for (int i = dataLen; i < *paddedLen; i++) {
        paddedData[i] = (unsigned char)paddingLength;
    }
    
    return paddedData;
}

unsigned char* encryptData(const unsigned char* data, int dataLen, const unsigned char* key, int* encryptedLen) {
    int paddedLen;
    unsigned char* paddedData = padData(data, dataLen, BLOCK_SIZE, &paddedLen);
    
    unsigned char* encryptedData = (unsigned char*)malloc(paddedLen);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    int len;
    EVP_EncryptUpdate(ctx, encryptedData, &len, paddedData, paddedLen);
    
    int finalLen;
    EVP_EncryptFinal_ex(ctx, encryptedData + len, &finalLen);
    
    *encryptedLen = len + finalLen;
    
    EVP_CIPHER_CTX_free(ctx);
    free(paddedData);
    
    return encryptedData;
}

char* toBase64(const unsigned char* data, int dataLen) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    int outputLen = ((dataLen + 2) / 3) * 4;
    char* result = (char*)malloc(outputLen + 1);
    int resultIdx = 0;
    
    int i = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    int arrayIdx = 0;
    
    for (int j = 0; j < dataLen; j++) {
        array3[arrayIdx++] = data[j];
        if (arrayIdx == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                result[resultIdx++] = base64_chars[array4[i]];
            arrayIdx = 0;
        }
    }
    
    if (arrayIdx) {
        for (int j = arrayIdx; j < 3; j++)
            array3[j] = '\\0';
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < arrayIdx + 1; j++)
            result[resultIdx++] = base64_chars[array4[j]];
        
        while (arrayIdx++ < 3)
            result[resultIdx++] = '=';
    }
    
    result[resultIdx] = '\\0';
    return result;
}

int main() {
    unsigned char key[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    
    printf("Test Case 1:\\n");
    const char* str1 = "Hello World!";
    int encryptedLen1;
    unsigned char* encrypted1 = encryptData((unsigned char*)str1, strlen(str1), key, &encryptedLen1);
    char* base64_1 = toBase64(encrypted1, encryptedLen1);
    printf("Encrypted: %s\\n", base64_1);
    free(encrypted1);
    free(base64_1);
    
    printf("\\nTest Case 2:\\n");
    const char* str2 = "Test";
    int encryptedLen2;
    unsigned char* encrypted2 = encryptData((unsigned char*)str2, strlen(str2), key, &encryptedLen2);
    char* base64_2 = toBase64(encrypted2, encryptedLen2);
    printf("Encrypted: %s\\n", base64_2);
    free(encrypted2);
    free(base64_2);
    
    printf("\\nTest Case 3:\\n");
    const char* str3 = "A";
    int encryptedLen3;
    unsigned char* encrypted3 = encryptData((unsigned char*)str3, strlen(str3), key, &encryptedLen3);
    char* base64_3 = toBase64(encrypted3, encryptedLen3);
    printf("Encrypted: %s\\n", base64_3);
    free(encrypted3);
    free(base64_3);
    
    printf("\\nTest Case 4:\\n");
    const char* str4 = "This is a longer message for encryption testing";
    int encryptedLen4;
    unsigned char* encrypted4 = encryptData((unsigned char*)str4, strlen(str4), key, &encryptedLen4);
    char* base64_4 = toBase64(encrypted4, encryptedLen4);
    printf("Encrypted: %s\\n", base64_4);
    free(encrypted4);
    free(base64_4);
    
    printf("\\nTest Case 5:\\n");
    const char* str5 = "1234567890123456";
    int encryptedLen5;
    unsigned char* encrypted5 = encryptData((unsigned char*)str5, strlen(str5), key, &encryptedLen5);
    char* base64_5 = toBase64(encrypted5, encryptedLen5);
    printf("Encrypted: %s\\n", base64_5);
    free(encrypted5);
    free(base64_5);
    
    return 0;
}
