
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <wincrypt.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
#endif

void generate_session_id(char* output, size_t output_size) {
    const size_t num_bytes = 32; // 256 bits
    unsigned char random_bytes[32];
    
    #ifdef _WIN32
        // Windows: Use CryptGenRandom
        HCRYPTPROV hCryptProv;
        if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            CryptGenRandom(hCryptProv, num_bytes, random_bytes);
            CryptReleaseContext(hCryptProv, 0);
        }
    #else
        // Unix/Linux: Use /dev/urandom
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            read(fd, random_bytes, num_bytes);
            close(fd);
        }
    #endif
    
    // Convert to hexadecimal string
    for (size_t i = 0; i < num_bytes && (i * 2 + 2) < output_size; ++i) {
        sprintf(output + (i * 2), "%02x", random_bytes[i]);
    }
    output[num_bytes * 2] = '\\0';
}

int main() {
    const size_t session_id_size = 65; // 32 bytes * 2 (hex) + 1 (null terminator)
    char session_id[session_id_size];
    
    printf("Generating 5 unique session IDs:\\n\\n");
    
    for (int i = 1; i <= 5; ++i) {
        generate_session_id(session_id, session_id_size);
        printf("Session ID %d: %s\\n", i, session_id);
        printf("Length: %zu characters\\n\\n", strlen(session_id));
    }
    
    return 0;
}
