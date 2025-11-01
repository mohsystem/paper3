#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// C-like struct person
struct Person {
    int id;               // 4 bytes
    char name[32];        // 32 bytes
    double salary;        // 8 bytes (alignment may add padding before this field)
};

// Portable secure zeroization
static void secure_zero(void* ptr, size_t len) {
    if (!ptr || len == 0) return;
#if defined(__STDC_LIB_EXT1__)
    (void)memset_s(ptr, len, 0, len);
#elif defined(_WIN32)
    SecureZeroMemory(ptr, len);
#else
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) { *p++ = 0; }
#endif
}

// Allocates a block of memory of size sizeof(struct Person) and returns a pointer
struct Person* allocate_person(void) {
    size_t count = 1;
    if (sizeof(struct Person) == 0 || count > (SIZE_MAX / sizeof(struct Person))) {
        return NULL;
    }
    struct Person* p = (struct Person*)calloc(count, sizeof(struct Person));
    return p; // may be NULL on failure
}

void free_person(struct Person* p) {
    if (p) {
        secure_zero(p, sizeof(*p));
        free(p);
    }
}

// Safe setters/getters
int set_id(struct Person* p, int id) {
    if (!p) return 0;
    p->id = id;
    return 1;
}

int set_name(struct Person* p, const char* name) {
    if (!p || !name) return 0;
    // Copy with bounds and ensure null termination
    // Using snprintf to prevent overflow
    (void)snprintf(p->name, sizeof(p->name), "%s", name);
    p->name[sizeof(p->name) - 1] = '\0';
    return 1;
}

int set_salary(struct Person* p, double salary) {
    if (!p) return 0;
    p->salary = salary;
    return 1;
}

int get_id(const struct Person* p) {
    return p ? p->id : 0;
}

const char* get_name(const struct Person* p) {
    return p ? p->name : "";
}

double get_salary(const struct Person* p) {
    return p ? p->salary : 0.0;
}

size_t sizeof_person(void) {
    return sizeof(struct Person);
}

int main(void) {
    // 5 test cases
    const char* names[5] = {
        "Alice",
        "Bob",
        "Carlos",
        "Diana",
        "Eve-LongName-ShouldBeTruncated-Beyond32"
    };
    int ids[5] = {1, 2, 3, 4, 5};
    double salaries[5] = {50000.0, 60000.5, 72000.75, 81000.0, 90500.25};

    printf("sizeof(person) = %zu bytes\n", sizeof_person());

    for (int i = 0; i < 5; ++i) {
        struct Person* p = allocate_person();
        if (!p) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }

        if (!set_id(p, ids[i]) || !set_name(p, names[i]) || !set_salary(p, salaries[i])) {
            fprintf(stderr, "Setting fields failed\n");
            free_person(p);
            return 1;
        }

        printf("Test %d: id=%d, name=\"%s\", salary=%.2f\n",
               i + 1, get_id(p), get_name(p), get_salary(p));

        free_person(p);
    }

    return 0;
}