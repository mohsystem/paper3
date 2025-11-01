#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#ifdef _MSC_VER
  #include <malloc.h>
  #define STACK_ALLOC _alloca
#else
  #include <alloca.h>
  #define STACK_ALLOC alloca
#endif

class Lse59 {
public:
    static std::string create_filled_stack_buffer(size_t buffer_size) {
        const size_t MAX_STACK_BUFFER = 64 * 1024; // 64 KiB safety limit
        if (buffer_size > MAX_STACK_BUFFER) {
            throw std::invalid_argument("buffer_size exceeds safe stack limit");
        }
        if (buffer_size == 0) {
            return std::string();
        }
        void* p = STACK_ALLOC(buffer_size);
        if (!p) {
            throw std::bad_alloc();
        }
        std::memset(p, 'A', buffer_size);
        return std::string(static_cast<char*>(p), buffer_size);
    }

    static void print_sample(const std::string& s) {
        std::cout << "len=" << s.size() << " sample=\"";
        size_t show = std::min<size_t>(20, s.size());
        for (size_t i = 0; i < show; ++i) std::cout << s[i];
        if (s.size() > show) std::cout << "...";
        std::cout << "\"\n";
    }

    static int main_cpp() {
        try {
            std::string t1 = create_filled_stack_buffer(0);
            std::string t2 = create_filled_stack_buffer(1);
            std::string t3 = create_filled_stack_buffer(5);
            std::string t4 = create_filled_stack_buffer(64);
            std::string t5 = create_filled_stack_buffer(1024);

            print_sample(t1);
            print_sample(t2);
            print_sample(t3);
            print_sample(t4);
            print_sample(t5);
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << "\n";
            return 1;
        }
        return 0;
    }
};

int main() {
    return Lse59::main_cpp();
}