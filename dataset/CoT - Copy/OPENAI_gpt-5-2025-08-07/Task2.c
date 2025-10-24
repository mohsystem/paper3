/* Chain-of-Through process in code generation:
   1) Problem understanding: Format exactly 10 integers (0-9) into "(XXX) XXX-XXXX".
   2) Security requirements: Validate inputs (null, length, range); avoid buffer overflows.
   3) Secure coding generation: Use checked formatting and dynamic allocation with size limits.
   4) Code review: Ensure proper error signaling and memory management.
   5) Secure code output: Final version with validation and 5 test cases. */

#include <stdio.h>
#include <stdlib.h>

/* Returns a dynamically allocated string "(XXX) XXX-XXXX" on success.
   On error, returns NULL and sets *err:
   0 = success, 1 = null input, 2 = invalid length, 3 = value out of range.
   Caller is responsible for freeing the returned string when not NULL. */
char* create_phone_number(const int* numbers, size_t len, int* err) {
    if (err) *err = 0;
    if (numbers == NULL) {
        if (err) *err = 1;
        return NULL;
    }
    if (len != 10) {
        if (err) *err = 2;
        return NULL;
    }
    for (size_t i = 0; i < 10; ++i) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            if (err) *err = 3;
            return NULL;
        }
    }
    // 14 characters + null terminator
    char* out = (char*)malloc(15);
    if (!out) {
        if (err) *err = 4; // allocation failure
        return NULL;
    }
    // Safe because each %d is a single digit after validation; total length = 14
    // snprintf writes at most 15 bytes including terminator.
    snprintf(out, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
             numbers[0], numbers[1], numbers[2],
             numbers[3], numbers[4], numbers[5],
             numbers[6], numbers[7], numbers[8], numbers[9]);
    return out;
}

int main(void) {
    // 5 test cases
    int err = 0;

    int t1[10] = {1,2,3,4,5,6,7,8,9,0};
    char* r1 = create_phone_number(t1, 10, &err);
    if (r1) { printf("Test 1: %s\n", r1); free(r1); } else { printf("Test 1 Error: %d\n", err); }

    int t2[10] = {0,1,2,3,4,5,6,7,8,9};
    char* r2 = create_phone_number(t2, 10, &err);
    if (r2) { printf("Test 2: %s\n", r2); free(r2); } else { printf("Test 2 Error: %d\n", err); }

    int t3[9]  = {0,1,2,3,4,5,6,7,8}; // invalid length
    char* r3 = create_phone_number(t3, 9, &err);
    if (r3) { printf("Test 3: %s\n", r3); free(r3); } else { printf("Test 3 Error: %d\n", err); }

    int t4[10] = {-1,1,2,3,4,5,6,7,8,9}; // invalid value
    char* r4 = create_phone_number(t4, 10, &err);
    if (r4) { printf("Test 4: %s\n", r4); free(r4); } else { printf("Test 4 Error: %d\n", err); }

    char* r5 = create_phone_number(NULL, 10, &err); // null input
    if (r5) { printf("Test 5: %s\n", r5); free(r5); } else { printf("Test 5 Error: %d\n", err); }

    return 0;
}