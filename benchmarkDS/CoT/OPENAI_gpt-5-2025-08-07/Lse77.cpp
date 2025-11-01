// Chain-of-Through secure implementation in C++
// 1) Problem: Safe memory allocation returning a pointer, and create a Person with status=0 and name="John".
// 2) Security: Validate size, cap allocations, zero memory, avoid overflows.
// 3) Secure coding: Defensive checks, nullptr on failure.
// 4) Review: Ensure safe string ops and memory handling.
// 5) Output: Final secure code.

#include <cstdlib>
#include <cstring>
#include <iostream>

static const size_t MAX_ALLOC = 64ULL * 1024ULL * 1024ULL; // 64 MB cap

struct Person {
    int status;
    char name[64];
};

void* secure_allocate(size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return nullptr;
    }
    void* ptr = std::malloc(size);
    if (!ptr) {
        return nullptr;
    }
    std::memset(ptr, 0, size);
    return ptr;
}

Person* create_person() {
    void* mem = secure_allocate(sizeof(Person));
    if (!mem) {
        return nullptr;
    }
    Person* p = static_cast<Person*>(mem);
    p->status = 0;
    std::strncpy(p->name, "John", sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    return p;
}

int main() {
    // 5 test cases for memory allocation
    size_t sizes[5] = {0, 16, 1024, static_cast<size_t>(-5), 200000000ULL};
    for (size_t s : sizes) {
        void* mem = secure_allocate(s);
        std::cout << "secure_allocate(" << s << ") -> " << (mem ? "non-null" : "null") << "\n";
        std::free(mem);
    }

    // 5 test cases for person creation
    for (int i = 0; i < 5; ++i) {
        Person* p = create_person();
        if (p) {
            std::cout << "Test " << (i + 1) << " create_person -> name='" << p->name << "', status=" << p->status << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " create_person -> null\n";
        }
        std::free(p);
    }
    return 0;
}