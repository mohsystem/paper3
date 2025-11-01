#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_ALLOC (1024u * 1024u) // 1 MiB cap
#define MAX_NAME_LEN 256u

typedef struct {
    int status;
    char name[MAX_NAME_LEN];
} Person;

// Memory allocation method: returns pointer to beginning of block or NULL on failure.
void* secure_alloc(size_t size) {
    if (size == 0 || size > (size_t)MAX_ALLOC) {
        return NULL;
    }
    void* p = malloc(size);
    if (p == NULL) {
        return NULL;
    }
    return p;
}

Person* create_person(const char* name, int status) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        return NULL;
    }
    p->status = status;

    const char* src = (name != NULL) ? name : "John";
    size_t max_copy = sizeof(p->name) - 1;
    size_t i = 0;
    while (i < max_copy && src[i] != '\0') {
        p->name[i] = src[i];
        ++i;
    }
    p->name[i] = '\0';
    return p;
}

void destroy_person(Person* p) {
    if (p != NULL) {
        free(p);
    }
}

int main(void) {
    // 5 test cases
    void* a1 = secure_alloc(16u);
    printf("%s\n", a1 ? "alloc 16 ok" : "alloc 16 failed");
    free(a1);

    void* a2 = secure_alloc(0u);
    printf("%s\n", a2 ? "alloc 0 ok" : "alloc 0 failed");
    free(a2);

    void* a3 = secure_alloc(MAX_ALLOC);
    printf("%s\n", a3 ? "alloc MAX ok" : "alloc MAX failed");
    free(a3);

    void* a4 = secure_alloc((size_t)MAX_ALLOC + 1u);
    printf("%s\n", a4 ? "alloc MAX+1 ok" : "alloc MAX+1 failed");
    free(a4);

    Person* person = create_person("John", 0);
    if (person != NULL) {
        printf("person: name='%s', status=%d\n", person->name, person->status);
        destroy_person(person);
    } else {
        printf("failed to create person\n");
    }

    return 0;
}