#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    char* str_a;
    char* str_b;
    char* str_c;
} StrTriple;

static uint32_t secure_random_u32(void) {
    uint32_t val = 0;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Failed to open /dev/urandom: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    size_t to_read = sizeof(val);
    unsigned char* p = (unsigned char*)&val;
    while (to_read > 0) {
        ssize_t n = read(fd, p, to_read);
        if (n < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "Failed to read /dev/urandom: %s\n", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            fprintf(stderr, "Unexpected EOF from /dev/urandom\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
        p += n;
        to_read -= (size_t)n;
    }
    if (close(fd) != 0) {
        fprintf(stderr, "Failed to close /dev/urandom: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return val;
}

static float secure_random_float(void) {
    uint32_t u = secure_random_u32();
    // Convert to [0.0f, 1.0f) using 24-bit mantissa for float
    // Use the top 24 bits to avoid bias due to float precision
    float f = (float)((u >> 8) & 0x00FFFFFFu) / 16777216.0f; // 2^24
    return f;
}

char* a(float value) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%.9g", value);
    if (len < 0 || (size_t)len >= sizeof(buf)) {
        return NULL;
    }
    char* out = (char*)malloc((size_t)len + 1U);
    if (!out) {
        return NULL;
    }
    memcpy(out, buf, (size_t)len);
    out[len] = '\0';
    return out;
}

StrTriple generate_random_float_strings(void) {
    float fa = secure_random_float();
    float fb = secure_random_float();
    float fc = secure_random_float();

    char* str_a = a(fa);
    char* str_b = a(fb);
    char* str_c = a(fc);

    StrTriple t = { str_a, str_b, str_c };
    return t;
}

int main(void) {
    for (int i = 0; i < 5; ++i) {
        StrTriple t = generate_random_float_strings();
        if (t.str_a && t.str_b && t.str_c) {
            printf("%s %s %s\n", t.str_a, t.str_b, t.str_c);
        } else {
            fprintf(stderr, "String conversion failed\n");
        }
        free(t.str_a);
        free(t.str_b);
        free(t.str_c);
    }
    return 0;
}