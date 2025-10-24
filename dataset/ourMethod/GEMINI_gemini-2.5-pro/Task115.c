#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Define a struct with bit fields.
// The total number of bits is 1 + 1 + 2 + 3 = 7 bits.
// The compiler will likely pack this into a single byte.
struct HardwareFlags {
    // LSB (Least Significant Bit) fields are often declared first
    // but the actual order is implementation-defined.
    unsigned int isActive : 1;
    unsigned int isReady : 1;
    unsigned int errorState : 2; // Can hold values 0-3
    unsigned int mode : 3;       // Can hold values 0-7
};

void print_flags(const struct HardwareFlags flags, int test_case_num) {
    printf("Test Case %d: isActive=%u, isReady=%u, errorState=%u, mode=%u\n",
           test_case_num,
           flags.isActive,
           flags.isReady,
           flags.errorState,
           flags.mode);
}

int main() {
    printf("--- C Bit Field Test Cases ---\n");
    printf("Size of HardwareFlags struct: %zu byte(s)\n", sizeof(struct HardwareFlags));

    // Test Case 1: All fields zero-initialized
    struct HardwareFlags flags1 = {0, 0, 0, 0};
    print_flags(flags1, 1);

    // Test Case 2: Setting individual fields
    struct HardwareFlags flags2 = {1, 0, 0, 5};
    print_flags(flags2, 2);

    // Test Case 3: Setting all fields to max values
    struct HardwareFlags flags3 = {1, 1, 3, 7};
    print_flags(flags3, 3);

    // Test Case 4: Setting and then changing fields
    struct HardwareFlags flags4 = {0, 1, 1, 4};
    printf("Test Case 4 (before): isActive=%u, isReady=%u, errorState=%u, mode=%u\n",
           flags4.isActive, flags4.isReady, flags4.errorState, flags4.mode);
    flags4.isReady = 0;
    flags4.errorState = 2;
    printf("Test Case 4 (after):  isActive=%u, isReady=%u, errorState=%u, mode=%u\n",
           flags4.isActive, flags4.isReady, flags4.errorState, flags4.mode);

    // Test Case 5: Partial initialization
    struct HardwareFlags flags5 = {true, true, 2, 0};
    print_flags(flags5, 5);

    return 0;
}