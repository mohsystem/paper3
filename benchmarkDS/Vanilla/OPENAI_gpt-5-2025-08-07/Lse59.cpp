#include <iostream>
#include <string>
#include <cstring>

#if defined(_MSC_VER)
  #include <malloc.h> // _malloca, _freea
#elif defined(__GNUC__) || defined(__clang__)
  #include <alloca.h> // alloca
#endif

// Allocates a buffer of size buffer_size on the stack (where supported) and overwrites with 'A'.
// Returns a std::string copy for safe use by the caller.
std::string fillBuffer(std::size_t buffer_size) {
    std::size_t n = buffer_size;
    if (n == 0) return std::string();

#if defined(_MSC_VER)
    char* buf = static_cast<char*>(_malloca(n + 1));
    std::memset(buf, 'A', n);
    buf[n] = '\0';
    std::string result(buf);
    _freea(buf);
    return result;
#elif defined(__GNUC__) || defined(__clang__)
    char* buf = static_cast<char*>(alloca(n + 1));
    std::memset(buf, 'A', n);
    buf[n] = '\0';
    return std::string(buf);
#else
    // Fallback: not strictly stack allocation.
    return std::string(n, 'A');
#endif
}

int main() {
    std::size_t tests[5] = {0, 1, 5, 10, 100};
    for (std::size_t t : tests) {
        std::string s = fillBuffer(t);
        std::cout << "size=" << t << " -> \"" << s << "\"\n";
    }
    return 0;
}