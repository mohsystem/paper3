#include <cstdio>
#include <cstdlib>
#include <cstring>

struct person {
    char name[50];
    int age;
    double height;
};

// Allocates a block of memory of size sizeof(person) and returns a pointer to the beginning
person* allocate_person() {
    person* p = static_cast<person*>(std::malloc(sizeof(person)));
    return p;
}

void set_person(person* p, const char* name, int age, double height) {
    if (!p) return;
    std::strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
    p->height = height;
}

void print_person(const person* p) {
    if (!p) return;
    std::printf("Person{name='%s', age=%d, height=%.2f, ptr=%p, size=%zu}\n",
                p->name, p->age, p->height, (const void*)p, sizeof(person));
}

int main() {
    // 5 test cases
    person* p1 = allocate_person();
    set_person(p1, "Alice", 30, 1.65);
    print_person(p1);

    person* p2 = allocate_person();
    set_person(p2, "Bob", 25, 1.80);
    print_person(p2);

    person* p3 = allocate_person();
    set_person(p3, "Carol", 40, 1.70);
    print_person(p3);

    person* p4 = allocate_person();
    set_person(p4, "Dave", 35, 1.75);
    print_person(p4);

    person* p5 = allocate_person();
    set_person(p5, "Eve", 28, 1.60);
    print_person(p5);

    // Free allocated memory
    std::free(p1);
    std::free(p2);
    std::free(p3);
    std::free(p4);
    std::free(p5);

    return 0;
}