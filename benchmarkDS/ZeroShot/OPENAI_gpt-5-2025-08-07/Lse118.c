#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
static int get_secure_random_bytes(void* buf, size_t len) {
    return BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0 ? 0 : -1;
}
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
static int get_secure_random_bytes(void* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, (unsigned char*)buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    return (off == len) ? 0 : -1;
}
#endif

static char* generate_and_write(const char* file_path) {
    uint32_t rnd[3];
    if (get_secure_random_bytes(rnd, sizeof(rnd)) != 0) {
        return NULL;
    }

    float a = (float)((double)rnd[0] / 4294967296.0);
    float b = (float)((double)rnd[1] / 4294967296.0);
    float c = (float)((double)rnd[2] / 4294967296.0);

    char temp[128];
    int n = snprintf(temp, sizeof(temp), "%.9g%.9g%.9g", a, b, c);
    if (n < 0 || n >= (int)sizeof(temp)) {
        return NULL;
    }

    char* result = (char*)malloc((size_t)n + 1);
    if (!result) return NULL;
    memcpy(result, temp, (size_t)n + 1);

    FILE* f = fopen(file_path, "wb");
    if (!f) {
        free(result);
        return NULL;
    }
    size_t written = fwrite(result, 1, (size_t)n, f);
    fclose(f);
    if (written != (size_t)n) {
        free(result);
        return NULL;
    }
    return result;
}

int main(void) {
    const char* files[5] = {"c_out1.txt", "c_out2.txt", "c_out3.txt", "c_out4.txt", "c_out5.txt"};
    for (int i = 0; i < 5; ++i) {
        char* s = generate_and_write(files[i]);
        if (s) {
            printf("Wrote to %s: %s\n", files[i], s);
            free(s);
        } else {
            printf("Error writing to %s\n", files[i]);
        }
    }
    return 0;
}