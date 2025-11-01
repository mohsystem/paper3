#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

Person* allocate_person(void) {
    Person* p = (Person*)calloc(1, sizeof(Person));
    if (!p) return NULL;
    p->status = 0;
    return p;
}

int main(void) {
    for (int i = 1; i <= 5; i++) {
        Person* p = allocate_person();
        if (!p) {
            printf("Allocation failed\n");
        } else {
            printf("Test %d: status=%d\n", i, p->status);
        }
        free(p);
    }
    return 0;
}