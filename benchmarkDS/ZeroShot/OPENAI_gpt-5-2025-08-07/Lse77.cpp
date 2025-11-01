#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <new>

static const std::size_t MAX_ALLOC_SIZE = 100ull * 1024ull * 1024ull; // 100 MiB cap

// Secure memory allocation: zero-initialized; returns nullptr on invalid size/failure.
void* allocate_memory(std::size_t size) {
    if (size == 0 || size > MAX_ALLOC_SIZE) {
        errno = EINVAL;
        return nullptr;
    }
    void* p = std::calloc(1, size);
    if (!p) {
        errno = ENOMEM;
    }
    return p;
}

struct Person {
    int status;
    std::string name;
};

// Creates a new person with status 0 and name "John"
Person* create_default_person() {
    Person* p = new (std::nothrow) Person();
    if (!p) {
        return nullptr;
    }
    p->status = 0;
    p->name = "John";
    return p;
}

int main() {
    void* a1 = allocate_memory(16);
    std::cout << "Test1 allocate 16 -> " << (a1 ? "ok" : "null") << "\n";
    std::free(a1);

    void* a2 = allocate_memory(0);
    std::cout << "Test2 allocate 0 -> " << (a2 ? "ok" : "null") << "\n";
    std::free(a2); // safe even if nullptr

    void* a3 = allocate_memory(1ull << 20);
    std::cout << "Test3 allocate 1_048_576 -> " << (a3 ? "ok" : "null") << "\n";
    std::free(a3);

    void* a4 = allocate_memory(static_cast<std::size_t>(-5)); // large due to wrap; will fail due to cap
    std::cout << "Test4 allocate -5(as size_t) -> " << (a4 ? "ok" : "null") << "\n";
    std::free(a4);

    Person* p = create_default_person();
    if (p) {
        std::cout << "Test5 create person -> {name: " << p->name << ", status: " << p->status << "}\n";
        delete p;
    } else {
        std::cout << "Test5 create person -> null\n";
    }

    return 0;
}