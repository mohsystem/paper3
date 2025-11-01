#include <iostream>
#include <memory>
#include <string>

static const std::size_t MAX_ALLOC = 1024u * 1024u; // 1 MiB cap
static const std::size_t MAX_NAME_LEN = 256u;

// Memory allocation method: returns pointer to beginning of block or empty on failure.
std::unique_ptr<unsigned char[]> allocate_memory(std::size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return {};
    }
    try {
        // Value-initialize to zero for safety
        return std::unique_ptr<unsigned char[]>(new unsigned char[size]());
    } catch (const std::bad_alloc&) {
        return {};
    }
}

struct Person {
    int status;
    std::string name;
};

Person create_person(const std::string& name, int status) {
    Person p;
    p.status = status;
    if (name.size() > MAX_NAME_LEN) {
        p.name = name.substr(0, MAX_NAME_LEN);
    } else {
        p.name = name;
    }
    return p;
}

int main() {
    // 5 test cases
    auto a1 = allocate_memory(16);
    std::cout << (a1 ? "alloc 16 ok" : "alloc 16 failed") << "\n";

    auto a2 = allocate_memory(0);
    std::cout << (a2 ? "alloc 0 ok" : "alloc 0 failed") << "\n";

    auto a3 = allocate_memory(MAX_ALLOC);
    std::cout << (a3 ? "alloc MAX ok" : "alloc MAX failed") << "\n";

    auto a4 = allocate_memory(MAX_ALLOC + 1);
    std::cout << (a4 ? "alloc MAX+1 ok" : "alloc MAX+1 failed") << "\n";

    Person p = create_person("John", 0);
    std::cout << "person: name='" << p.name << "', status=" << p.status << "\n";
    return 0;
}