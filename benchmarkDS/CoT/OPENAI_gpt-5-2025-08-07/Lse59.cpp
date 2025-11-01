#include <iostream>
#include <string>
#include <cstring>

#if defined(_MSC_VER)
  #include <malloc.h>
#else
  #include <alloca.h>
#endif

// Step 1-4: Implement secure overwrite with bounds checking
std::string overwrite_with_A(std::size_t buffer_size) {
    const std::size_t MAX_STACK_BUFFER = 65536; // Security: prevent excessive stack usage
    if (buffer_size == 0) return std::string();
    if (buffer_size > MAX_STACK_BUFFER) return std::string();

    // Allocate on stack safely with bounded size
    std::size_t alloc_size = buffer_size > 0 ? buffer_size : 1;
    char* buf = static_cast<char*>(alloca(alloc_size));
    if (!buf) return std::string();
    std::memset(buf, 'A', buffer_size);
    return std::string(buf, buffer_size);
}

int main() {
    std::size_t tests[5] = {0, 1, 5, 16, 70000};
    for (std::size_t t : tests) {
        std::string result = overwrite_with_A(t);
        std::cout << "bufferSize=" << t << ", len=" << result.size()
                  << ", preview=" << (result.size() > 16 ? result.substr(0,16) : result)
                  << "\n";
    }
    return 0;
}