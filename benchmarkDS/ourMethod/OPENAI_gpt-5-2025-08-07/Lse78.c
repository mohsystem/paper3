#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_ALLOC (64 * 1024 * 1024) /* 64MB cap */
#define MAX_NAME_LEN 128

typedef struct Person {
    char *name;
    int age;
    int status; /* set to 0 */
} Person;

/* Safe, bounded string length */
static size_t bounded_strlen(const char *s, size_t max) {
    size_t n = 0;
    if (s == NULL) return 0;
    while (n < max && s[n] != '\0') {
        n++;
    }
    return n;
}

/* Memory allocation: returns pointer to beginning of block or NULL on failure */
void *allocate_memory(size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return NULL;
    }
    void *p = malloc(size);
    return p; /* may be NULL if allocation failed */
}

/* Allocates a new Person and sets the status to 0 */
Person *allocate_person(const char *name, int age) {
    if (name == NULL) {
        return NULL;
    }
    if (age < 0 || age > 150) {
        return NULL;
    }

    size_t nlen = bounded_strlen(name, MAX_NAME_LEN);
    /* Check for overflow on nlen + 1 */
    if (nlen > SIZE_MAX - 1) {
        return NULL;
    }

    Person *p = (Person *)calloc(1, sizeof(Person));
    if (p == NULL) {
        return NULL;
    }

    p->name = (char *)malloc(nlen + 1);
    if (p->name == NULL) {
        free(p);
        return NULL;
    }

    if (nlen > 0) {
        memcpy(p->name, name, nlen);
    }
    p->name[nlen] = '\0';

    p->age = age;
    p->status = 0; /* explicitly set status to 0 */

    return p;
}

void free_person(Person **pp) {
    if (pp == NULL || *pp == NULL) return;
    Person *p = *pp;
    if (p->name != NULL) {
        /* name does not hold secrets; direct free is fine */
        free(p->name);
        p->name = NULL;
    }
    free(p);
    *pp = NULL;
}

static void print_person(const Person *p) {
    if (p == NULL) {
        printf("Person: (null)\n");
        return;
    }
    const char *nm = (p->name != NULL) ? p->name : "(null)";
    printf("Person{name='%s', age=%d, status=%d}\n", nm, p->age, p->status);
}

static void preview_mem(const void *ptr, size_t size) {
    if (ptr == NULL) {
        printf("Memory: null\n");
        return;
    }
    const unsigned char *b = (const unsigned char *)ptr;
    size_t n = size < 8 ? size : 8;
    printf("Memory: len=%zu, head=[", size);
    for (size_t i = 0; i < n; ++i) {
        printf("%u%s", (unsigned)b[i], (i + 1 < n) ? ", " : "");
    }
    printf("]\n");
}

int main(void) {
    /* 5 test cases: allocate a person and a memory block each time */
    const char *names[5] = {
        "Alice",
        "Bob",
        "Charlie",
        "ThisIsAVeryLongNameThatWillBeClippedBecauseItExceedsTheMaximumAllowedLengthABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "  Eve  "
    };
    int ages[5] = {30, 25, 40, 22, 28};
    size_t sizes[5] = {16, 0, 1024, MAX_ALLOC, (size_t)MAX_ALLOC + 1};

    for (int i = 0; i < 5; ++i) {
        /* Trim simple leading/trailing spaces for the test before allocation */
        const char *raw = names[i];
        const char *start = raw;
        while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;
        const char *end = raw + strlen(raw);
        while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end--;
        size_t len = (size_t)(end - start);
        char temp[256];
        if (len >= sizeof(temp)) len = sizeof(temp) - 1;
        if (len > 0) {
            memcpy(temp, start, len);
        }
        temp[len] = '\0';

        Person *p = allocate_person(temp, ages[i]);
        if (p != NULL) {
            print_person(p);
        } else {
            printf("Person alloc failed\n");
        }

        void *mem = allocate_memory(sizes[i]);
        if (mem != NULL) {
            if (sizes[i] >= 2) {
                unsigned char *b = (unsigned char *)mem;
                b[0] = (unsigned char)i;
                b[1] = (unsigned char)(i * 2);
            }
            preview_mem(mem, sizes[i]);
            free(mem);
        } else {
            printf("Memory alloc failed\n");
        }

        free_person(&p);
    }

    return 0;
}