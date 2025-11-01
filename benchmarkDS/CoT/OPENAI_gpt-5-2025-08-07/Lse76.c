/* Chain-of-Through process:
 * 1) Problem understanding: Allocate a Person block and set status to 0.
 * 2) Security requirements: Check malloc result; avoid undefined behavior; free memory.
 * 3) Secure coding: Validate pointers; initialize fields explicitly.
 * 4) Code review: No buffer overflows; no unchecked dereferences.
 * 5) Secure code output: Final code with 5 test cases and cleanup.
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

Person* allocate_person(void) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        return NULL;
    }
    p->status = 0;
    return p;
}

void free_person(Person* p) {
    free(p);
}

int main(void) {
    Person* people[5] = {NULL, NULL, NULL, NULL, NULL};

    for (int i = 0; i < 5; ++i) {
        people[i] = allocate_person();
    }

    for (int i = 0; i < 5; ++i) {
        if (people[i] != NULL) {
            printf("Test %d - Person status: %d\n", i + 1, people[i]->status);
        } else {
            printf("Test %d - Allocation failed\n", i + 1);
        }
    }

    for (int i = 0; i < 5; ++i) {
        free_person(people[i]);
    }

    return 0;
}