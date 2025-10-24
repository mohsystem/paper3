#include <iostream>
#include <bitset>
#include <stdexcept>

// A struct representing a 16-bit hardware register using bit fields.
// The layout of bit fields in memory is implementation-defined, but this
// demonstrates the concept. Using 'unsigned int' is generally portable.
struct HardwareRegister {
    // Fields are specified from least significant bit to most significant bit
    unsigned int enable     : 1;
    unsigned int ready      : 1;
    unsigned int mode       : 2;
    unsigned int error_code : 4;
    unsigned int reserved   : 8;

    // Encapsulated methods to provide secure access and validation
    void setMode(int m) {
        if (m < 0 || m > 3) {
            throw std::out_of_range("Mode must be between 0 and 3.");
        }
        mode = m;
    }

    void setErrorCode(int code) {
        if (code < 0 || code > 15) {
            throw std::out_of_range("Error code must be between 0 and 15.");
        }
        error_code = code;
    }
};

// A union to easily access the raw integer value of the struct
union RegisterUnion {
    HardwareRegister fields;
    uint16_t raw; // Use a fixed-size integer type
};

void print_register(const RegisterUnion& reg_union) {
    std::bitset<16> bits(reg_union.raw);
    std::cout << "Register(binary=" << bits << ") -> "
              << "enable=" << reg_union.fields.enable
              << ", ready=" << reg_union.fields.ready
              << ", mode=" << reg_union.fields.mode
              << ", errorCode=" << reg_union.fields.error_code
              << ", reserved=" << reg_union.fields.reserved
              << std::endl;
}

int main() {
    std::cout << "--- C++ Bit Fields ---" << std::endl;

    // Test Case 1: Initial state (zero-initialized)
    std::cout << "\n--- Test Case 1: Initial State ---" << std::endl;
    RegisterUnion reg1 = {0};
    print_register(reg1);

    // Test Case 2: Setting individual fields
    std::cout << "\n--- Test Case 2: Setting Individual Fields ---" << std::endl;
    RegisterUnion reg2 = {0};
    reg2.fields.enable = 1;
    reg2.fields.setMode(3); // 11 in binary
    reg2.fields.setErrorCode(10); // 1010 in binary
    print_register(reg2);

    // Test Case 3: Setting all fields
    std::cout << "\n--- Test Case 3: Setting All Fields ---" << std::endl;
    RegisterUnion reg3 = {0};
    reg3.fields.enable = 1;
    reg3.fields.ready = 1;
    reg3.fields.setMode(1);
    reg3.fields.setErrorCode(5);
    reg3.fields.reserved = 255;
    print_register(reg3);

    // Test Case 4: Modifying existing fields
    std::cout << "\n--- Test Case 4: Modifying Existing Fields ---" << std::endl;
    reg3.fields.enable = 0;
    reg3.fields.setErrorCode(15);
    print_register(reg3);

    // Test Case 5: Invalid input
    std::cout << "\n--- Test Case 5: Invalid Input ---" << std::endl;
    RegisterUnion reg5 = {0};
    try {
        reg5.fields.setMode(4);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    try {
        reg5.fields.setErrorCode(-1);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << "State after invalid attempts: ";
    print_register(reg5);

    return 0;
}