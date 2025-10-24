#include <stdio.h>
#include <stdint.h>
#include <string.h>

// A struct representing a 16-bit hardware register using bit fields.
// Using 'unsigned int' is standard, but the exact memory layout
// can be implementation-defined.
typedef struct {
    unsigned int enable     : 1;
    unsigned int ready      : 1;
    unsigned int mode       : 2;
    unsigned int error_code : 4;
    unsigned int reserved   : 8;
} HardwareRegister;

// A union to allow accessing the struct's data as a single integer,
// which is useful for printing or serialization.
typedef union {
    HardwareRegister fields;
    uint16_t raw; // Use a fixed-width integer for predictability.
} RegisterUnion;

// Helper function to print the binary representation of the register.
void print_binary(uint16_t n) {
    char binaryStr[17];
    binaryStr[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        binaryStr[15 - i] = ((n >> i) & 1) ? '1' : '0';
    }
    printf("%s", binaryStr);
}

// Function to print the state of the register.
void print_register(RegisterUnion reg_union) {
    printf("Register(binary=");
    print_binary(reg_union.raw);
    printf(") -> enable=%u, ready=%u, mode=%u, errorCode=%u, reserved=%u\n",
           reg_union.fields.enable,
           reg_union.fields.ready,
           reg_union.fields.mode,
           reg_union.fields.error_code,
           reg_union.fields.reserved);
}

int main() {
    printf("--- C Bit Fields ---\n");
    
    // Test Case 1: Initial state (zero-initialized)
    printf("\n--- Test Case 1: Initial State ---\n");
    RegisterUnion reg1;
    reg1.raw = 0; // Explicitly zero out the entire union.
    print_register(reg1);

    // Test Case 2: Setting individual fields
    printf("\n--- Test Case 2: Setting Individual Fields ---\n");
    RegisterUnion reg2;
    reg2.raw = 0;
    reg2.fields.enable = 1;
    reg2.fields.mode = 3;     // 11 in binary
    reg2.fields.error_code = 10; // 1010 in binary
    print_register(reg2);

    // Test Case 3: Setting all fields
    printf("\n--- Test Case 3: Setting All Fields ---\n");
    RegisterUnion reg3;
    reg3.raw = 0;
    reg3.fields.enable = 1;
    reg3.fields.ready = 1;
    reg3.fields.mode = 1;
    reg3.fields.error_code = 5;
    reg3.fields.reserved = 255;
    print_register(reg3);
    
    // Test Case 4: Modifying existing fields
    printf("\n--- Test Case 4: Modifying Existing Fields ---\n");
    reg3.fields.enable = 0;
    reg3.fields.error_code = 15;
    print_register(reg3);
    
    // Test Case 5: Out-of-range assignment (behavior is truncation)
    printf("\n--- Test Case 5: Out-of-range Assignment ---\n");
    RegisterUnion reg5;
    reg5.raw = 0;
    // Assigning 4 (binary 100) to a 2-bit field. The value will be truncated
    // to 0 (binary 00). Compilers often warn about this.
    reg5.fields.mode = 4;
    printf("After setting mode to 4 (which truncates): ");
    print_register(reg5);
    
    return 0;
}