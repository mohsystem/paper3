#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <string>
#include <iostream>

// C-like struct person
struct Person {
    int id;               // 4 bytes
    char name[32];        // 32 bytes
    double salary;        // 8 bytes (alignment may add padding before this field)
};

// Secure zeroization
static void secure_zero(void* ptr, size_t len) {
#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#else
    if (!ptr || len == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) { *p++ = 0; }
#endif
}

// Allocates a block of memory of size sizeof(Person) and returns a pointer
Person* allocate_person() {
    size_t count = 1;
    if (sizeof(Person) == 0 || count > (SIZE_MAX / sizeof(Person))) {
        return nullptr;
    }
    void* mem = std::calloc(count, sizeof(Person));
    return static_cast<Person*>(mem); // may be nullptr on failure
}

void free_person(Person* p) {
    if (p) {
        secure_zero(p, sizeof(Person));
        std::free(p);
    }
}

// Safe setters/getters
bool set_id(Person* p, int id) {
    if (!p) return false;
    p->id = id;
    return true;
}

bool set_name(Person* p, const char* name) {
    if (!p || !name) return false;
    // Copy with bounds and ensure null termination
    std::snprintf(p->name, sizeof(p->name), "%s", name);
    p->name[sizeof(p->name) - 1] = '\0';
    return true;
}

bool set_salary(Person* p, double salary) {
    if (!p) return false;
    p->salary = salary;
    return true;
}

int get_id(const Person* p) {
    return p ? p->id : 0;
}

std::string get_name(const Person* p) {
    if (!p) return std::string();
    // name is null-terminated due to snprintf use
    return std::string(p->name);
}

double get_salary(const Person* p) {
    return p ? p->salary : 0.0;
}

size_t sizeof_person() {
    return sizeof(Person);
}

int main() {
    // 5 test cases
    const char* names[5] = {
        "Alice",
        "Bob",
        "Carlos",
        "Diana",
        "Eve-LongName-ShouldBeTruncated-Beyond32"
    };
    int ids[5] = {1, 2, 3, 4, 5};
    double salaries[5] = {50000.0, 60000.5, 72000.75, 81000.0, 90500.25};

    std::cout << "sizeof(person) = " << sizeof_person() << " bytes\n";

    for (int i = 0; i < 5; ++i) {
        Person* p = allocate_person();
        if (!p) {
            std::cerr << "Allocation failed\n";
            return 1;
        }

        set_id(p, ids[i]);
        set_name(p, names[i]);
        set_salary(p, salaries[i]);

        std::cout << "Test " << (i + 1)
                  << ": id=" << get_id(p)
                  << ", name=\"" << get_name(p) << "\""
                  << ", salary=" << get_salary(p)
                  << "\n";

        free_person(p);
    }

    return 0;
}