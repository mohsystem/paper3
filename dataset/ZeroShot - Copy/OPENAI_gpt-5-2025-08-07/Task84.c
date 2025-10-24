#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#elif defined(__linux__)
  #include <sys/random.h>
  #include <unistd.h>
  #include <fcntl.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
#endif

static int secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#elif defined(__linux__)
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(buf + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += (size_t)r;
    }
    if (off == len) return 1;
    int fd = open("/dev/urandom", O_RDONLY
    #ifdef O_CLOEXEC
        | O_CLOEXEC
    #endif
    );
    if (fd < 0) return 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) { close(fd); return 0; }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#else
    size_t off = 0;
    int fd = open("/dev/urandom", O_RDONLY
    #ifdef O_CLOEXEC
        | O_CLOEXEC
    #endif
    );
    if (fd < 0) return 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) { close(fd); return 0; }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static char* to_hex(const uint8_t* data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    out[len * 2] = '\0';
    return out;
}

/* Simple chained hash set for strings to enforce uniqueness */
typedef struct Node {
    char* id;
    struct Node* next;
} Node;

#define TABLE_SIZE 4096
static Node* table[TABLE_SIZE] = {0};

static uint64_t fnv1a_hash(const char* s) {
    uint64_t h = 1469598103934665603ULL;
    while (*s) {
        h ^= (unsigned char)(*s++);
        h *= 1099511628211ULL;
    }
    return h;
}

static int set_contains(const char* id) {
    uint64_t h = fnv1a_hash(id);
    size_t idx = (size_t)(h % TABLE_SIZE);
    Node* cur = table[idx];
    while (cur) {
        if (strcmp(cur->id, id) == 0) return 1;
        cur = cur->next;
    }
    return 0;
}

static int set_add(const char* id) {
    if (set_contains(id)) return 0;
    uint64_t h = fnv1a_hash(id);
    size_t idx = (size_t)(h % TABLE_SIZE);
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    size_t len = strlen(id);
    n->id = (char*)malloc(len + 1);
    if (!n->id) { free(n); return 0; }
    memcpy(n->id, id, len + 1);
    n->next = table[idx];
    table[idx] = n;
    return 1;
}

static size_t clamp_length(size_t n) {
    if (n < 16 || n > 64) return 32;
    return n;
}

char* generate_session_id(size_t num_bytes) {
    size_t n = clamp_length(num_bytes);
    uint8_t* buf = (uint8_t*)malloc(n);
    if (!buf) return NULL;

    for (;;) {
        if (!secure_random_bytes(buf, n)) {
            free(buf);
            return NULL;
        }
        char* id = to_hex(buf, n);
        if (!id) {
            free(buf);
            return NULL;
        }
        if (set_add(id)) {
            free(buf);
            return id; // caller must free
        }
        free(id); // collision, regenerate
    }
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        char* id = generate_session_id(32);
        if (!id) {
            fprintf(stderr, "Failed to generate session ID\n");
            return 1;
        }
        printf("%s\n", id);
        free(id);
    }
    return 0;
}