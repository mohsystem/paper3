#include <stdio.h>
#include <stdint.h>  // For fixed-width integers like uint8_t
#include <stdbool.h> // For the bool type

// C supports bit fields directly within a struct. This is a common
// technique in embedded systems for memory-efficient hardware register mapping.
struct StatusRegister {
    // Specify the number of bits for each member after the colon.
    // Using unsigned types is recommended for predictable behavior.
    uint8_t active : 1;
    uint8_t mode : 3;
    uint8_t error_code : 4;
};

// A union allows us to interpret the same block of memory in multiple ways.
// Here, we can access the data via the 'fields' struct or as a single 'raw_value' byte.
union Status {
    struct StatusRegister fields;
    uint8_t raw_value;
};

/**
 * @brief Prints the binary representation of a byte.
 * @param n The byte to print.
 */
void print_binary(uint8_t n) {
    for (int i = 7; i >= 0; i--) {
        putchar((n & (1 << i)) ? '1' : '0');
    }
}

/**
 * @brief Creates a Status union, initializes its bit fields, and prints the result.
 * @param active boolean flag (1 bit).
 * @param mode integer value from 0-7 (3 bits).
 * @param error_code integer value from 0-15 (4 bits).
 */
void createAndPrintStatus(bool active, unsigned int mode, unsigned int error_code) {
    if (mode > 7 || error_code > 15) {
        printf("Error: Invalid input values.\n");
        return;
    }
    
    union Status status;
    status.raw_value = 0; // It's crucial to initialize the memory to zero first.

    status.fields.active = active;
    status.fields.mode = mode;
    status.fields.error_code = error_code;

    printf("Input: active=%d, mode=%u, error_code=%u\n", active, mode, error_code);
    printf("Packed Value: %u (Binary: ", status.raw_value);
    print_binary(status.raw_value);
    printf(")\n");
    printf("  -> Unpacked: active=%u, mode=%u, error_code=%u\n",
           status.fields.active, status.fields.mode, status.fields.error_code);
}

int main() {
    printf("C Bit Fields:\n");
    // The total size is packed by the compiler, often into a single byte for this example.
    printf("Size of StatusRegister struct: %zu byte(s)\n", sizeof(struct StatusRegister));

    // --- Test Cases ---
    printf("\n--- Test Case 1 ---\n");
    createAndPrintStatus(true, 5, 10);

    printf("\n--- Test Case 2 ---\n");
    createAndPrintStatus(false, 0, 0);

    printf("\n--- Test Case 3 ---\n");
    createAndPrintStatus(true, 7, 15);

    printf("\n--- Test Case 4 ---\n");
    createAndPrintStatus(false, 1, 8);

    printf("\n--- Test Case 5 ---\n");
    createAndPrintStatus(true, 3, 4);

    return 0;
}