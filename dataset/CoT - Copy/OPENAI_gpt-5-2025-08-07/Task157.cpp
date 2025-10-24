#include <iostream>
#include <new>
#include <climits>
#include <cstddef>

static const std::size_t MAX_SIZE = 16u * 1024u * 1024u;

static void secure_memzero(void* p, std::size_t n) {
    volatile unsigned char* vp = static_cast<volatile unsigned char*>(p);
    while (n--) {
        *vp++ = 0;
    }
}

// Dynamically allocates a memory buffer, uses it, securely clears it, and deallocates it.
// Returns checksum on success; ULLONG_MAX on error (invalid size or OOM).
unsigned long long allocate_and_use(std::size_t size) {
    if (size == 0 || size > MAX_SIZE) {
        return ULLONG_MAX;
    }
    unsigned char* buf = new (std::nothrow) unsigned char[size];
    if (!buf) {
        return ULLONG_MAX;
    }

    for (std::size_t i = 0; i < size; ++i) {
        buf[i] = static_cast<unsigned char>(i & 0xFF);
    }

    unsigned long long checksum = 0ULL;
    for (std::size_t i = 0; i < size; ++i) {
        checksum += static_cast<unsigned long long>(buf[i]);
    }

    secure_memzero(buf, size);
    delete[] buf;
    buf = nullptr;

    return checksum;
}

int main() {
    std::size_t tests[5] = {1, 64, 1024, 4096, 1000000};
    for (std::size_t t : tests) {
        unsigned long long res = allocate_and_use(t);
        std::cout << "size=" << t << " checksum=" << res << "\n";
    }
    return 0;
}