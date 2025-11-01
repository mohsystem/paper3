#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int status;
} Person;

// Allocates a new Person and ensures status is set to 0.
Person* allocate_person(int /*ignored_status*/) {
    Person* p = (Person*)calloc(1, sizeof(Person));
    if (p == NULL) {
        return NULL;
    }
    p->status = 0;
    return p;
}

int main(void) {
    int tests[5] = { -1, 0, 1, 42, INT_MAX };
    for (int i = 0; i < 5; ++i) {
        Person* p = allocate_person(tests[i]);
        if (p == NULL) {
            printf("Test %d: allocation failed\n", i + 1);
        } else {
            printf("Test %d: input=%d status=%d\n", i + 1, tests[i], p->status);
            free(p);
        }
    }
    return 0;
}