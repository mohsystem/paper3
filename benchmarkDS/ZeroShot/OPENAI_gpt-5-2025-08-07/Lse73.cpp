#include <iostream>
#include <cstring>
#include <new>
#include <limits>

struct Person {
    char  name[64];  // fixed-size for safe bounded copy
    int   age;
    double height;
};

Person* allocate_person() {
    // Value-initialize to zero the memory; returns pointer to the start of the block
    Person* p = new (std::nothrow) Person{};
    return p; // nullptr if allocation failed
}

bool set_person(Person* p, const char* name, int age, double height) {
    if (!p) return false;
    if (age < 0 || age > 150) return false;
    if (!(height > 0.0 && height < 3.0)) return false;
    if (!name) name = "";

    // Safe, bounded copy with explicit null-termination
    std::size_t maxlen = sizeof(p->name) - 1;
    std::strncpy(p->name, name, maxlen);
    p->name[maxlen] = '\0';

    p->age = age;
    p->height = height;
    return true;
}

void deallocate_person(Person* p) {
    delete p;
}

void print_person(const Person* p) {
    if (!p) {
        std::cout << "Person{null}\n";
        return;
    }
    std::cout << "Person{name='" << p->name << "', age=" << p->age << ", height=" << p->height << "}\n";
}

int main() {
    const char* names[5] = {
        "Alice",
        "BobTheBuilderWithAReallyLongNameThatExceedsSixtyThreeCharacters_ABCDEFG",
        "",
        "Zhang Wei",
        "Eve"
    };
    int ages[5] = {30, 45, 20, 28, 35};
    double heights[5] = {1.65, 1.80, 1.70, 1.75, 1.82};

    for (int i = 0; i < 5; ++i) {
        Person* p = allocate_person();
        bool ok = set_person(p, names[i], ages[i], heights[i]);
        std::cout << "Allocation " << (i + 1) << " success=" << (ok ? "true" : "false") << " -> ";
        print_person(p);
        deallocate_person(p);
    }
    return 0;
}