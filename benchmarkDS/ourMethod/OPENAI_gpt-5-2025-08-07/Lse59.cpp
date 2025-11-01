#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <limits>

#if defined(_MSC_VER)
  #include <malloc.h> // _alloca
  #define stack_alloc _alloca
#else
  #include <alloca.h> // alloca
  #define stack_alloc alloca
#endif

static constexpr size_t MAX_STACK = 1u * 1024u * 1024u; // 1 MiB cap to avoid stack exhaustion

// Allocates a buffer of size bufferSize on the stack and fills with 'A'.
// Returns a std::string copy of the buffer (safe to return).
std::string fillBufferOnStack(size_t bufferSize) {
    if (bufferSize > MAX_STACK) {
        throw std::invalid_argument("invalid size");
    }
    if (bufferSize == 0) {
        return std::string();
    }
    // Allocate on stack safely with cap
    void* p = stack_alloc(bufferSize);
    if (!p) {
        throw std::runtime_error("stack allocation failed");
    }
    char* buf = static_cast<char*>(p);
    std::memset(buf, 'A', bufferSize);
    return std::string(buf, bufferSize);
}

int main() {
    size_t tests[5] = {0, 1, 5, 16, 1024};
    for (size_t sz : tests) {
        try {
            std::string out = fillBufferOnStack(sz);
            std::string sample = out.size() <= 32 ? out : (out.substr(0, 32) + "...");
            std::cout << "size=" << sz << " len=" << out.size() << " sample=" << sample << "\n";
        } catch (const std::exception& ex) {
            std::cout << "size=" << sz << " error=" << ex.what() << "\n";
        }
    }
    return 0;
}