#include <stdio.h>
#include <stdlib.h>

struct person {
    int status;
};

// Allocates a block of memory of size sizeof(struct person) and returns a pointer
struct person* allocate_person(void) {
    struct person* p = (struct person*)malloc(sizeof(struct person));
    if (p != NULL) {
        p->status = 0;
    }
    return p;
}

int main(void) {
    // 5 test cases
    struct person* p1 = allocate_person();
    struct person* p2 = allocate_person();
    struct person* p3 = allocate_person();
    struct person* p4 = allocate_person();
    struct person* p5 = allocate_person();

    if (p1) printf("p1->status = %d\n", p1->status);
    if (p2) printf("p2->status = %d\n", p2->status);
    if (p3) printf("p3->status = %d\n", p3->status);
    if (p4) printf("p4->status = %d\n", p4->status);
    if (p5) printf("p5->status = %d\n", p5->status);

    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);

    return 0;
}