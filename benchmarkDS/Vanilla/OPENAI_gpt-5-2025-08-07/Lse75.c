#include <stdio.h>
#include <stdlib.h>

struct person {
    int status;
};

// Allocates a block of memory of size sizeof(struct person) and returns a pointer.
// Sets status to 0.
struct person* alloc_person(void) {
    struct person* p = (struct person*)malloc(sizeof(struct person));
    if (p) {
        p->status = 0;
    }
    return p;
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        struct person* p = alloc_person();
        if (p) {
            printf("person{status=%d}\n", p->status);
            free(p);
        } else {
            printf("Allocation failed\n");
        }
    }
    return 0;
}