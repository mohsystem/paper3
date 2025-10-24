#include <iostream>
#include <bitset>   // For printing binary representation
#include <cstdint>  // For fixed-width integers like uint8_t

// C++ supports bit fields directly within a struct or class.
// This allows the compiler to pack data tightly.
struct StatusRegister {
    // We specify the number of bits for each member.
    // Using unsigned types is best practice to avoid sign-extension issues.
    uint8_t active : 1;
    uint8_t mode : 3;
    uint8_t error_code : 4;
};

// A union is a helpful way to access the same memory in different ways:
// either through the bit fields struct or as a single raw byte.
union Status {
    StatusRegister fields;
    uint8_t raw_value;
};

/**
 * Creates a Status union, initializes its bit fields, and prints the result.
 * @param active bool flag (1 bit).
 * @param mode integer value from 0-7 (3 bits).
 * @param error_code integer value from 0-15 (4 bits).
 */
void createAndPrintStatus(bool active, unsigned int mode, unsigned int error_code) {
    if (mode > 7 || error_code > 15) {
        std::cout << "Error: Invalid input values." << std::endl;
        return;
    }
    
    Status status;
    status.raw_value = 0; // Initialize all bits to 0 before setting fields.

    status.fields.active = active;
    status.fields.mode = mode;
    status.fields.error_code = error_code;
    
    std::cout << "Input: active=" << active << ", mode=" << mode << ", error_code=" << error_code << std::endl;
    std::cout << "Packed Value: " << static_cast<int>(status.raw_value)
              << " (Binary: " << std::bitset<8>(status.raw_value) << ")" << std::endl;
    std::cout << "  -> Unpacked: active=" << status.fields.active
              << ", mode=" << status.fields.mode
              << ", error_code=" << status.fields.error_code << std::endl;
}

int main() {
    std::cout << "C++ Bit Fields:" << std::endl;
    // The size is often 1 byte because 1+3+4 = 8 bits.
    std::cout << "Size of StatusRegister struct: " << sizeof(StatusRegister) << " byte(s)" << std::endl;
    
    // --- Test Cases ---
    std::cout << "\n--- Test Case 1 ---" << std::endl;
    createAndPrintStatus(true, 5, 10);

    std::cout << "\n--- Test Case 2 ---" << std::endl;
    createAndPrintStatus(false, 0, 0);

    std::cout << "\n--- Test Case 3 ---" << std::endl;
    createAndPrintStatus(true, 7, 15);
    
    std::cout << "\n--- Test Case 4 ---" << std::endl;
    createAndPrintStatus(false, 1, 8);
    
    std::cout << "\n--- Test Case 5 ---" << std::endl;
    createAndPrintStatus(true, 3, 4);

    return 0;
}