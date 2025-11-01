#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

typedef struct {
    int status;
} Person;

Person* allocate_person(void) {
    // Allocate a block of memory of size sizeof(Person) and set status to 0
    size_t sz = sizeof(Person);
    if (sz == 0 || sz > SIZE_MAX) {
        return NULL;
    }
    Person* p = (Person*)malloc(sz);
    if (p == NULL) {
        return NULL;
    }
    p->status = 0;
    return p;
}

int main(void) {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        Person* p = allocate_person();
        if (p == NULL) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }
        printf("Test %d: status=%d\n", i, p->status);
        free(p);
    }
    return 0;
}