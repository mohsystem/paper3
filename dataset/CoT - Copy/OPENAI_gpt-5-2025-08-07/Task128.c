/* Chain-of-Through Process:
1) Problem understanding: Provide secure random numbers within range and secure tokens.
2) Security requirements: Use OS CSPRNG (Windows CNG or POSIX getrandom/urandom). Validate inputs and avoid modulo bias.
3) Secure coding generation: Rejection sampling for uniformity, base64url without padding, careful memory handling.
4) Code review: Check for integer overflow, handle partial reads, and resource cleanup.
5) Secure code output: Final code adheres to security practices with 5 test cases.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #ifdef __linux__
    #include <sys/random.h>
  #endif
  #include <errno.h>
#endif

static int secure_random_bytes(unsigned char* buffer, size_t len) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buffer, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#else
  #ifdef __linux__
    size_t total = 0;
    while (total < len) {
        ssize_t r = getrandom(buffer + total, len - total, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        total += (size_t)r;
    }
    if (total == len) return 1;
  #endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, buffer + total, len - total);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) {
            close(fd);
            return 0;
        }
        total += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static int secure_random_in_range(int minVal, int maxVal, int* outVal) {
    if (minVal > maxVal || outVal == NULL) return 0;
    uint64_t range = (uint64_t)((int64_t)maxVal - (int64_t)minVal) + 1ULL;
    uint64_t limit = ~((uint64_t)0) - (~((uint64_t)0) % range);
    for (;;) {
        uint64_t x = 0;
        if (!secure_random_bytes((unsigned char*)&x, sizeof(x))) return 0;
        if (x < limit) {
            *outVal = minVal + (int)(x % range);
            return 1;
        }
    }
}

static int* generate_random_numbers(size_t count, int minVal, int maxVal) {
    if (minVal > maxVal) return NULL;
    int* arr = NULL;
    if (count > 0) {
        arr = (int*)malloc(count * sizeof(int));
        if (!arr) return NULL;
    } else {
        // return empty array (NULL indicates no data; caller should check count)
        return NULL;
    }
    for (size_t i = 0; i < count; ++i) {
        int v = 0;
        if (!secure_random_in_range(minVal, maxVal, &v)) {
            free(arr);
            return NULL;
        }
        arr[i] = v;
    }
    return arr;
}

// Base64URL encoding without padding
static char* base64url_encode(const unsigned char* data, size_t len) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t full_groups = len / 3;
    size_t rem = len % 3;
    size_t out_len = full_groups * 4 + (rem ? (rem + 1) : 0);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    for (size_t g = 0; g < full_groups; ++g) {
        unsigned int b0 = data[i++];
        unsigned int b1 = data[i++];
        unsigned int b2 = data[i++];
        out[j++] = alphabet[(b0 >> 2) & 0x3F];
        out[j++] = alphabet[((b0 & 0x3) << 4) | ((b1 >> 4) & 0x0F)];
        out[j++] = alphabet[((b1 & 0x0F) << 2) | ((b2 >> 6) & 0x03)];
        out[j++] = alphabet[b2 & 0x3F];
    }

    if (rem == 1) {
        unsigned int b0 = data[i++];
        out[j++] = alphabet[(b0 >> 2) & 0x3F];
        out[j++] = alphabet[(b0 & 0x3) << 4];
    } else if (rem == 2) {
        unsigned int b0 = data[i++];
        unsigned int b1 = data[i++];
        out[j++] = alphabet[(b0 >> 2) & 0x3F];
        out[j++] = alphabet[((b0 & 0x3) << 4) | ((b1 >> 4) & 0x0F)];
        out[j++] = alphabet[(b1 & 0x0F) << 2];
    }

    out[j] = '\0';
    return out;
}

static char* generate_secure_token(size_t byteLength) {
    if (byteLength == 0) return NULL;
    unsigned char* buf = (unsigned char*)malloc(byteLength);
    if (!buf) return NULL;
    if (!secure_random_bytes(buf, byteLength)) {
        free(buf);
        return NULL;
    }
    char* token = base64url_encode(buf, byteLength);
    free(buf);
    return token;
}

static void print_array(const int* arr, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // 1) 5 numbers between 1 and 10
    printf("Test1 numbers [1,10]:\n");
    size_t n1 = 5;
    int* a = generate_random_numbers(n1, 1, 10);
    if (a) { print_array(a, n1); free(a); } else { printf("Error generating numbers\n"); }

    // 2) Secure token with 16 bytes
    printf("Test2 token (16 bytes):\n");
    char* t1 = generate_secure_token(16);
    if (t1) { printf("%s\n", t1); free(t1); } else { printf("Error generating token\n"); }

    // 3) 10 numbers between -50 and 50
    printf("Test3 numbers [-50,50]:\n");
    size_t n3 = 10;
    int* b = generate_random_numbers(n3, -50, 50);
    if (b) { print_array(b, n3); free(b); } else { printf("Error generating numbers\n"); }

    // 4) Secure token with 32 bytes
    printf("Test4 token (32 bytes):\n");
    char* t2 = generate_secure_token(32);
    if (t2) { printf("%s\n", t2); free(t2); } else { printf("Error generating token\n"); }

    // 5) 3 numbers in [0,0]
    printf("Test5 numbers [0,0]:\n");
    size_t n5 = 3;
    int* c = generate_random_numbers(n5, 0, 0);
    if (c) { print_array(c, n5); free(c); } else { printf("Error generating numbers\n"); }

    return 0;
}