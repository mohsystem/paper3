#include <stdio.h>
#include <stdlib.h>

struct person {
    int status;
};

struct person* allocate_person(void) {
    struct person* p = (struct person*)malloc(sizeof(struct person));
    if (p != NULL) {
        p->status = 0;
    }
    return p;
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        struct person* p = allocate_person();
        if (p != NULL) {
            printf("%d\n", p->status);
            free(p);
        } else {
            printf("allocation_failed\n");
        }
    }
    return 0;
}