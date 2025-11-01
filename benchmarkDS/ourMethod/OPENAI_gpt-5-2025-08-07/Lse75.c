#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN 64
#endif

typedef struct Person {
    char name[MAX_NAME_LEN];
    int age;
    int status; /* explicitly set to 0 on allocation */
} Person;

static void secure_zero(void *v, size_t n) {
    if (!v || n == 0) return;
#if defined(__STDC_LIB_EXT1__)
    (void)memset_s(v, n, 0, n);
#else
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
#endif
}

/* Safely copy string with bounds checking and null termination */
static int safe_copy(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) return -1;
    if (!src) src = "unknown";
    int written = snprintf(dst, dst_size, "%s", src);
    if (written < 0) return -1;
    if ((size_t)written >= dst_size) {
        /* Truncation occurred, but snprintf already null-terminated */
    }
    return 0;
}

/* Allocates a block of memory of size sizeof(Person) and returns a pointer to it.
   Sets status to 0. Returns NULL on failure or invalid input. */
Person* allocate_person(const char *name, int age) {
    if (age < 0 || age > 150) {
        return NULL; /* Fail closed on invalid age */
    }

    Person *p = (Person *)calloc(1u, sizeof(Person));
    if (p == NULL) {
        return NULL;
    }

    const char *safe_name = (name && name[0] != '\0') ? name : "unknown";
    if (safe_copy(p->name, sizeof(p->name), safe_name) != 0) {
        secure_zero(p, sizeof(Person));
        free(p);
        return NULL;
    }

    p->age = age;
    p->status = 0; /* explicitly set status to 0 */

    return p;
}

void free_person(Person *p) {
    if (p == NULL) return;
    secure_zero(p->name, sizeof(p->name));
    secure_zero(p, sizeof(Person));
    free(p);
}

int main(void) {
    /* 5 test cases */
    Person *p1 = allocate_person("Alice", 30);
    Person *p2 = allocate_person("Bob", 45);
    Person *p3 = allocate_person("Charlie", 0);
    Person *p4 = allocate_person("Dana", 99);
    Person *p5 = allocate_person(NULL, 25);

    if (p1) { printf("Person{name='%s', age=%d, status=%d}\n", p1->name, p1->age, p1->status); }
    else { printf("Allocation failed\n"); }
    if (p2) { printf("Person{name='%s', age=%d, status=%d}\n", p2->name, p2->age, p2->status); }
    else { printf("Allocation failed\n"); }
    if (p3) { printf("Person{name='%s', age=%d, status=%d}\n", p3->name, p3->age, p3->status); }
    else { printf("Allocation failed\n"); }
    if (p4) { printf("Person{name='%s', age=%d, status=%d}\n", p4->name, p4->age, p4->status); }
    else { printf("Allocation failed\n"); }
    if (p5) { printf("Person{name='%s', age=%d, status=%d}\n", p5->name, p5->age, p5->status); }
    else { printf("Allocation failed\n"); }

    free_person(p1);
    free_person(p2);
    free_person(p3);
    free_person(p4);
    free_person(p5);

    return 0;
}