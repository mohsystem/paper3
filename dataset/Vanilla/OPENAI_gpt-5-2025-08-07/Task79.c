#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encrypt(const char* message, const char* key) {
    size_t mlen = strlen(message);
    size_t klen = strlen(key);
    if (klen == 0) return NULL;
    char* hex = (char*)malloc(mlen * 2 + 1);
    if (!hex) return NULL;
    const char* HEX = "0123456789ABCDEF";
    for (size_t i = 0; i < mlen; ++i) {
        unsigned char c = ((unsigned char)message[i]) ^ ((unsigned char)key[i % klen]);
        hex[2 * i]     = HEX[(c >> 4) & 0x0F];
        hex[2 * i + 1] = HEX[c & 0x0F];
    }
    hex[mlen * 2] = '\0';
    return hex;
}

int main() {
    const char* msgs[5] = {
        "HELLO WORLD",
        "Attack at dawn!",
        "OpenAI",
        "The quick brown fox jumps over the lazy dog.",
        "Lorem ipsum dolor sit amet"
    };
    const char* keys[5] = {
        "KEY",
        "secret",
        "GPT",
        "cipher",
        "xyz"
    };

    for (int i = 0; i < 5; ++i) {
        char* enc = encrypt(msgs[i], keys[i]);
        if (enc) {
            printf("%s\n", enc);
            free(enc);
        } else {
            printf("Encryption failed (possibly empty key or alloc failure)\n");
        }
    }
    return 0;
}