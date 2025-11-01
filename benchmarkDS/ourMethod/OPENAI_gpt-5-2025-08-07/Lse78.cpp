#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <limits>

static constexpr std::size_t MAX_ALLOC = 64 * 1024 * 1024; // 64MB
static constexpr std::size_t MAX_NAME_LEN = 128;

struct Person {
    std::string name;
    int age{0};
    int status{0}; // status set to 0
};

// Memory allocation: returns pointer to beginning of block or nullptr on failure
void* allocate_memory(std::size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return nullptr;
    }
    void* p = std::malloc(size);
    return p; // may be nullptr if allocation failed
}

// Allocates a new Person and sets the status to 0
std::unique_ptr<Person> allocate_person(const std::string& name, int age) {
    if (age < 0 || age > 150) {
        return nullptr;
    }
    std::string nm = name;
    // trim leading/trailing spaces
    auto l = nm.find_first_not_of(" \t\r\n");
    auto r = nm.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) {
        nm.clear();
    } else {
        nm = nm.substr(l, r - l + 1);
    }
    if (nm.size() > MAX_NAME_LEN) {
        nm.resize(MAX_NAME_LEN);
    }
    auto p = std::make_unique<Person>();
    p->name = nm;
    p->age = age;
    p->status = 0;
    return p;
}

static std::string preview(void* ptr, std::size_t size) {
    if (!ptr) return "null";
    unsigned char* b = static_cast<unsigned char*>(ptr);
    std::size_t n = size < 8 ? size : 8;
    std::string s = "len=" + std::to_string(size) + ", head=[";
    for (std::size_t i = 0; i < n; ++i) {
        s += std::to_string(static_cast<unsigned int>(b[i]));
        if (i + 1 < n) s += ", ";
    }
    s += "]";
    return s;
}

int main() {
    // 5 test cases
    std::vector<std::string> names = {
        "Alice",
        "Bob",
        "Charlie",
        "ThisIsAVeryLongNameThatWillBeClippedBecauseItExceedsTheMaximumAllowedLengthABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "  Eve  "
    };
    std::vector<int> ages = {30, 25, 40, 22, 28};
    std::vector<std::size_t> sizes = {16, 0, 1024, MAX_ALLOC, MAX_ALLOC + 1};

    for (int i = 0; i < 5; ++i) {
        auto person = allocate_person(names[i], ages[i]);
        if (person) {
            std::cout << "Created: Person{name='" << person->name
                      << "', age=" << person->age
                      << ", status=" << person->status << "}\n";
        } else {
            std::cout << "Person alloc failed\n";
        }

        void* mem = allocate_memory(sizes[i]);
        if (mem) {
            // Write a couple bytes safely
            if (sizes[i] >= 2) {
                static_cast<unsigned char*>(mem)[0] = static_cast<unsigned char>(i);
                static_cast<unsigned char*>(mem)[1] = static_cast<unsigned char>(i * 2);
            }
            std::cout << "Memory: " << preview(mem, sizes[i]) << "\n";
            std::free(mem);
        } else {
            std::cout << "Memory alloc failed\n";
        }
    }

    return 0;
}