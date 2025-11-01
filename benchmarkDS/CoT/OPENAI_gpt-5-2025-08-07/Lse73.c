/* Chain-of-Through process:
 * 1) Problem understanding: Implement a function that allocates memory sized to sizeof(person) and returns a pointer.
 * 2) Security requirements: Validate size and allocation result, avoid undefined behavior, and free after use.
 * 3) Secure coding generation: Use malloc/calloc with checks and return NULL on failure.
 * 4) Code review: Verified error handling and cleanup.
 * 5) Secure code output: Safe C implementation with 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct person {
    int id;
    double salary;
    char name[16];
};

void* allocate_person_memory(int zero_init) {
    size_t sz = sizeof(struct person);
    if (sz == 0) {
        return NULL;
    }
    void* p = zero_init ? calloc(1, sz) : malloc(sz);
    if (!p) {
        return NULL;
    }
    return p;
}

int main(void) {
    int tests[5] = { 1, 0, 1, 0, 1 };
    for (int i = 0; i < 5; ++i) {
        void* block = allocate_person_memory(tests[i]);
        if (!block) {
            printf("Test %d (zero_init=%s): allocation failed\n", i + 1, tests[i] ? "true" : "false");
        } else {
            printf("Test %d (zero_init=%s): ptr=%p, size=%zu\n", i + 1, tests[i] ? "true" : "false", block, sizeof(struct person));
            free(block);
        }
    }
    return 0;
}