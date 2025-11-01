/* Chain-of-Through process in code generation:
   1) Problem understanding: Build a function to allocate memory and a function to create a person with status=0.
   2) Security requirements: Validate size, cap allocation, and handle failure paths safely.
   3) Secure coding generation: Use calloc for zero-initialization and return NULL on invalid inputs.
   4) Code review: Ensure freeing of resources and correct bounds checks.
   5) Secure code output: Final code with safe practices implemented. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static const size_t MAX_ALLOC = 64u * 1024u * 1024u; /* 64 MiB */

void* allocate_memory(size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return NULL;
    }
    /* calloc provides zero-initialized memory and checks for overflow internally on some platforms */
    void* ptr = calloc(1u, size);
    return ptr; /* may be NULL on failure */
}

typedef struct {
    int status;
} Person;

Person* new_person(void) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p != NULL) {
        p->status = 0;
    }
    return p;
}

int main(void) {
    size_t tests[5] = { 0u, 8u, 1024u, 10000u, (size_t)(MAX_ALLOC + 1u) };
    for (int i = 0; i < 5; ++i) {
        size_t s = tests[i];
        void* mem = allocate_memory(s);
        Person* p = new_person();
        long long cap = (mem != NULL) ? (long long)s : -1;
        int person_status = (p != NULL) ? p->status : -1;
        printf("Test %d | size=%zu | memAllocated=%s | capacity=%lld | person.status=%d\n",
               i + 1, s, (mem != NULL) ? "1" : "0", cap, person_status);
        free(mem);
        free(p);
    }
    return 0;
}