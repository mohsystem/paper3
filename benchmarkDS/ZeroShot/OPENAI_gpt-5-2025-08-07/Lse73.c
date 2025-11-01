#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

struct person {
    char name[64];
    int age;
    double height;
};

struct person* allocate_person(void) {
    // Allocates a zero-initialized block of memory of size sizeof(struct person)
    // and returns a pointer to the beginning of that block.
    if (sizeof(struct person) == 0) {
        return NULL;
    }
    struct person* p = (struct person*)calloc(1, sizeof(struct person));
    return p; // NULL on failure
}

bool set_person(struct person* p, const char* name, int age, double height) {
    if (p == NULL) return false;
    if (age < 0 || age > 150) return false;
    if (!(height > 0.0 && height < 3.0)) return false;

    if (name == NULL) name = "";
    // Safe bounded copy with guaranteed null-termination
    size_t maxlen = sizeof(p->name) - 1;
    strncpy(p->name, name, maxlen);
    p->name[maxlen] = '\0';

    p->age = age;
    p->height = height;
    return true;
}

void print_person(const struct person* p) {
    if (p == NULL) {
        printf("Person{null}\n");
        return;
    }
    printf("Person{name='%s', age=%d, height=%.2f}\n", p->name, p->age, p->height);
}

int main(void) {
    const char* names[5] = {
        "Alice",
        "BobTheBuilderWithAReallyLongNameThatExceedsSixtyThreeCharacters_ABCDEFG",
        "",
        "Zhang Wei",
        "Eve"
    };
    int ages[5] = {30, 45, 20, 28, 35};
    double heights[5] = {1.65, 1.80, 1.70, 1.75, 1.82};

    for (int i = 0; i < 5; ++i) {
        struct person* p = allocate_person();
        bool ok = set_person(p, names[i], ages[i], heights[i]);
        printf("Allocation %d success=%s -> ", i + 1, ok ? "true" : "false");
        print_person(p);
        free(p);
    }
    return 0;
}