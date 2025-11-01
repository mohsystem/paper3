#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

void* allocateMemory(size_t size) {
    return malloc(size);
}

Person* allocatePerson(void) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p) p->status = 0;
    return p;
}

int main(void) {
    size_t sizes[5] = {0, 1, 16, 1024, 65536};

    // 5 test cases for memory allocation
    for (int i = 0; i < 5; ++i) {
        void* mem = allocateMemory(sizes[i]);
        printf("Allocated memory pointer: %p size: %zu\n", mem, sizes[i]);
        free(mem);
    }

    // 5 test cases for person allocation
    for (int i = 0; i < 5; ++i) {
        Person* p = allocatePerson();
        printf("Person status: %d\n", p ? p->status : -1);
        free(p);
    }

    return 0;
}