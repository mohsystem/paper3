#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

Person* allocate_person(void) {
    // Allocates a new Person and sets status to 0.
    Person* p = (Person*)calloc(1, sizeof(Person));
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
            fprintf(stderr, "Allocation failed at test %d\n", i);
        } else {
            printf("Test %d: status=%d\n", i, p->status);
            free(p);
            p = NULL;
        }
    }
    return 0;
}