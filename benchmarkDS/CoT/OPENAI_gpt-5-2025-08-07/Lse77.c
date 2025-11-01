/* Chain-of-Through secure implementation in C
   1) Problem: Safe memory allocation returning a pointer, and create a Person with status=0 and name="John".
   2) Security: Validate size, cap allocations, zero memory, avoid overflows.
   3) Secure coding: Defensive checks, NULL on failure.
   4) Review: Safe string operations and memory handling.
   5) Output: Final secure code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_ALLOC (64U * 1024U * 1024U) /* 64 MB cap */

typedef struct {
    int status;
    char name[64];
} Person;

void* secure_allocate(size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return NULL;
    }
    void* ptr = malloc(size);
    if (ptr == NULL) {
        return NULL;
    }
    memset(ptr, 0, size);
    return ptr;
}

Person* create_person() {
    void* mem = secure_allocate(sizeof(Person));
    if (mem == NULL) {
        return NULL;
    }
    Person* p = (Person*)mem;
    p->status = 0;
    strncpy(p->name, "John", sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    return p;
}

int main(void) {
    /* 5 test cases for memory allocation */
    size_t sizes[5] = {0U, 16U, 1024U, (size_t)-5, 200000000U};
    for (int i = 0; i < 5; ++i) {
        size_t s = sizes[i];
        void* mem = secure_allocate(s);
        printf("secure_allocate(%zu) -> %s\n", s, mem ? "non-null" : "null");
        free(mem);
    }

    /* 5 test cases for person creation */
    for (int i = 0; i < 5; ++i) {
        Person* p = create_person();
        if (p != NULL) {
            printf("Test %d create_person -> name='%s', status=%d\n", i + 1, p->name, p->status);
        } else {
            printf("Test %d create_person -> null\n", i + 1);
        }
        free(p);
    }

    return 0;
}