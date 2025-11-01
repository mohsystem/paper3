#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static const size_t MAX_SAFE_SIZE = 16u * 1024u * 1024u; // 16 MB cap

typedef struct {
    int status;
} Person;

// Secure memory allocation: returns pointer to zeroed block or NULL on error
void* allocate_memory(size_t size) {
    if (size == 0 || size > MAX_SAFE_SIZE) {
        return NULL;
    }
    void* p = calloc(1, size); // zero-initialized
    return p; // NULL if allocation failed
}

// Allocates a new Person and sets status to 0
Person* allocate_person(void) {
    Person* p = (Person*)calloc(1, sizeof(Person)); // zero-initialized
    if (p != NULL) {
        p->status = 0;
    }
    return p;
}

int main(void) {
    // 5 test cases

    // Test 1: Allocate 1024 bytes
    void* t1 = allocate_memory(1024);
    printf("Test1 allocate_memory(1024): %s\n", t1 != NULL ? "ok" : "null");
    free(t1);

    // Test 2: Allocate Person and verify status
    Person* p1 = allocate_person();
    printf("Test2 allocate_person(): status=%d\n", p1 != NULL ? p1->status : -1);
    free(p1);

    // Test 3: Allocate 0 bytes (invalid per guard)
    void* t3 = allocate_memory(0);
    printf("Test3 allocate_memory(0): %s\n", t3 != NULL ? "ok" : "null");
    free(t3);

    // Test 4: Allocate excessively large size
    void* t4 = allocate_memory(MAX_SAFE_SIZE + 1);
    printf("Test4 allocate_memory(>MAX): %s\n", t4 != NULL ? "ok" : "null");
    free(t4);

    // Test 5: Another valid allocation
    void* t5 = allocate_memory(2048);
    printf("Test5 allocate_memory(2048): %s\n", t5 != NULL ? "ok" : "null");
    free(t5);

    return 0;
}