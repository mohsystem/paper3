#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 64

typedef struct person {
    char name[MAX_NAME_LEN + 1]; /* +1 for NUL terminator */
    int age;
    int status; /* set to 0 on allocation */
} person;

static int sanitize_age(int age) {
    if (age < 0) return 0;
    if (age > 150) return 150;
    return age;
}

person* allocate_person(const char* name, int age) {
    person* p = NULL;

    /* Allocate exactly sizeof(person) bytes and zero-initialize */
    p = (person*)calloc(1, sizeof(person));
    if (p == NULL) {
        return NULL; /* Allocation failed */
    }

    /* Set status explicitly to 0 (calloc already zeroes, but be explicit) */
    p->status = 0;

    /* Sanitize age */
    p->age = sanitize_age(age);

    /* Safely copy name into bounded buffer */
    if (name == NULL) {
        p->name[0] = '\0';
    } else {
        /* Trim leading/trailing spaces (simple, safe approach) */
        const char* start = name;
        while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
            start++;
        }
        const char* end = start + strlen(start);
        while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
            end--;
        }
        size_t trimmed_len = (size_t)(end - start);
        if (trimmed_len > MAX_NAME_LEN) trimmed_len = MAX_NAME_LEN;

        /* Copy exactly trimmed_len bytes and NUL-terminate */
        if (trimmed_len > 0) {
            memcpy(p->name, start, trimmed_len);
        }
        p->name[trimmed_len] = '\0';
    }

    return p; /* returns pointer to start of allocated block */
}

static void print_person(const person* p) {
    if (p == NULL) {
        printf("Allocation failed\n");
        return;
    }
    printf("Person{name='%s', age=%d, status=%d}\n", p->name, p->age, p->status);
}

int main(void) {
    /* 5 test cases */
    person* p1 = allocate_person("Alice", 30);
    person* p2 = allocate_person(" Bob  ", -5);
    person* p3 = allocate_person(NULL, 151);
    person* p4 = allocate_person("ThisNameIsWayTooLongAndShouldBeTruncatedAtSixtyFourCharacters_ABCDEFGHIJKLMNOPQRSTUVWXYZ", 42);
    person* p5 = allocate_person("   ", 0);

    print_person(p1);
    print_person(p2);
    print_person(p3);
    print_person(p4);
    print_person(p5);

    /* Free allocated memory exactly once */
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);

    return 0;
}