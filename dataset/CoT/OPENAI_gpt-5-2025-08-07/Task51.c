/* Chain-of-Through secure implementation for a simple Caesar cipher.
   1) Problem understanding: Encrypt a given string using Caesar cipher.
   2) Security requirements: Handle NULL input, avoid buffer overflows, check allocations.
   3) Secure coding generation: Normalize key, allocate exact buffer, preserve non-alpha.
   4) Code review: Validate modulo arithmetic and memory operations.
   5) Secure code output: Final, checked implementation. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int normalize_key(int key) {
    int k = key % 26;
    if (k < 0) k += 26;
    return k;
}

char* encrypt(const char* text, int key) {
    size_t len = 0;
    if (text != NULL) {
        len = strlen(text);
    }
    if (len > SIZE_MAX - 1) {
        return NULL; /* overflow guard */
    }
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    int k = normalize_key(key);
    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        if (c >= 'A' && c <= 'Z') {
            int offset = c - 'A';
            out[i] = (char)('A' + (offset + k) % 26);
        } else if (c >= 'a' && c <= 'z') {
            int offset = c - 'a';
            out[i] = (char)('a' + (offset + k) % 26);
        } else {
            out[i] = c;
        }
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* inputs[5] = {
        "Hello, World!",
        "abcxyz ABCXYZ",
        "Attack at dawn! 123",
        "",
        "Zebra-123"
    };
    int keys[5] = {3, 2, 5, 10, -1};
    for (int i = 0; i < 5; i++) {
        char* enc = encrypt(inputs[i], keys[i]);
        if (enc == NULL) {
            printf("Encryption failed for test %d\n", i + 1);
        } else {
            printf("Input: %s | Key: %d | Encrypted: %s\n", inputs[i], keys[i], enc);
            free(enc);
        }
    }
    return 0;
}