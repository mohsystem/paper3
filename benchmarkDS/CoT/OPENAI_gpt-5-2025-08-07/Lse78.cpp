// Chain-of-Through process in code generation:
// 1) Problem understanding: Provide memory allocation and person creation with status set to 0.
// 2) Security requirements: Validate size, cap allocation, handle failures, and avoid undefined behavior.
// 3) Secure coding generation: Use calloc for zero-initialized memory, return nullptr on failure.
// 4) Code review: Ensure proper free/delete and checks; avoid integer overflow by capping size.
// 5) Secure code output: Final code includes safe patterns and thorough checks.

#include <iostream>
#include <cstdlib>
#include <cstdint>

static const std::size_t MAX_ALLOC = 64ull * 1024ull * 1024ull; // 64 MiB

void* allocate_memory(std::size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return nullptr;
    }
    void* ptr = std::calloc(1, size); // zero-initialized
    return ptr; // nullptr on failure
}

struct Person {
    int status;
};

Person* new_person() {
    Person* p = static_cast<Person*>(std::malloc(sizeof(Person)));
    if (p != nullptr) {
        p->status = 0;
    }
    return p;
}

int main() {
    std::size_t tests[5] = { 0, 8, 1024, 10000, MAX_ALLOC + 1 };
    for (int i = 0; i < 5; ++i) {
        std::size_t s = tests[i];
        void* mem = allocate_memory(s);
        Person* p = new_person();
        std::cout << "Test " << (i + 1)
                  << " | size=" << s
                  << " | memAllocated=" << (mem != nullptr)
                  << " | capacity=" << ((mem != nullptr) ? static_cast<long long>(s) : -1)
                  << " | person.status=" << (p ? p->status : -1)
                  << "\n";
        if (mem) std::free(mem);
        if (p) std::free(p);
    }
    return 0;
}