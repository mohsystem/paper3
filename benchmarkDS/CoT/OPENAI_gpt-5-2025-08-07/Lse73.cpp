// Chain-of-Through process:
// 1) Problem understanding: Provide a function that allocates memory of size sizeof(person) and returns a pointer to its beginning.
// 2) Security requirements: Validate size, check allocation result, and avoid undefined behavior. Properly free memory in tests.
// 3) Secure coding generation: Use malloc/calloc with checks and return nullptr on failure.
// 4) Code review: Ensured pointer checks and frees, no buffer overflows.
// 5) Secure code output: Final, safe implementation with 5 test cases.

#include <cstdlib>
#include <cstdio>
#include <new>

struct person {
    int id;
    double salary;
    char name[16];
};

void* allocate_person_memory(bool zero_init) {
    size_t sz = sizeof(person);
    if (sz == 0) {
        return nullptr;
    }
    void* p = zero_init ? std::calloc(1, sz) : std::malloc(sz);
    if (!p) {
        return nullptr; // caller must handle failure
    }
    return p;
}

int main() {
    bool tests[5] = { true, false, true, false, true };
    for (int i = 0; i < 5; ++i) {
        void* block = allocate_person_memory(tests[i]);
        if (!block) {
            std::printf("Test %d (zero_init=%s): allocation failed\n", i + 1, tests[i] ? "true" : "false");
        } else {
            std::printf("Test %d (zero_init=%s): ptr=%p, size=%zu\n", i + 1, tests[i] ? "true" : "false", block, sizeof(person));
            std::free(block);
        }
    }
    return 0;
}