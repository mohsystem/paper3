#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char* name;
} Person;

void* alloc(size_t size) {
    return malloc(size);
}

Person* create_person(const char* name, int status) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (!p) return NULL;
    p->status = status;
    if (name) {
        p->name = (char*)malloc(strlen(name) + 1);
        if (!p->name) {
            free(p);
            return NULL;
        }
        strcpy(p->name, name);
    } else {
        p->name = NULL;
    }
    return p;
}

void free_person(Person* p) {
    if (!p) return;
    free(p->name);
    free(p);
}

int main(void) {
    size_t sizes[5] = {8, 16, 32, 64, 128};
    void* blocks[5] = {NULL};

    for (int i = 0; i < 5; ++i) {
        blocks[i] = alloc(sizes[i]);
        printf("Allocated block %d of size %zu at address %p\n", i + 1, sizes[i], blocks[i]);
    }

    Person* john = create_person("John", 0);
    if (john) {
        printf("Created person: name=%s, status=%d\n", john->name ? john->name : "(null)", john->status);
    } else {
        printf("Failed to create person\n");
    }

    // Additional small validations
    if (blocks[0] && sizes[0] >= 3) {
        unsigned char* p = (unsigned char*)blocks[0];
        p[0] = 1; p[1] = 2; p[2] = 3;
        printf("Sample bytes: %u,%u,%u\n", p[0], p[1], p[2]);
    }

    for (int i = 0; i < 5; ++i) {
        free(blocks[i]);
    }
    free_person(john);
    return 0;
}