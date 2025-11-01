#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_ALLOC_SIZE (100u * 1024u * 1024u) /* 100 MiB cap */

void* allocate_memory(size_t size) {
    if (size == 0 || size > MAX_ALLOC_SIZE) {
        errno = EINVAL;
        return NULL;
    }
    void* p = calloc(1, size); /* zero-initialized */
    if (!p) {
        errno = ENOMEM;
    }
    return p;
}

typedef struct {
    int status;
    char name[64];
} Person;

/* Creates a new person with status 0 and name "John" */
Person* create_default_person(void) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (!p) {
        return NULL;
    }
    p->status = 0;
    /* Secure copy with explicit NUL-termination */
    strncpy(p->name, "John", sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    return p;
}

int main(void) {
    void* a1 = allocate_memory(16);
    printf("Test1 allocate 16 -> %s\n", a1 ? "ok" : "null");
    free(a1);

    void* a2 = allocate_memory(0);
    printf("Test2 allocate 0 -> %s\n", a2 ? "ok" : "null");
    free(a2);

    void* a3 = allocate_memory(1024u * 1024u);
    printf("Test3 allocate 1_048_576 -> %s\n", a3 ? "ok" : "null");
    free(a3);

    void* a4 = allocate_memory((size_t)-5); /* will exceed cap -> NULL */
    printf("Test4 allocate -5(as size_t) -> %s\n", a4 ? "ok" : "null");
    free(a4);

    Person* p = create_default_person();
    if (p) {
        printf("Test5 create person -> {name: %s, status: %d}\n", p->name, p->status);
        free(p);
    } else {
        printf("Test5 create person -> null\n");
    }

    return 0;
}