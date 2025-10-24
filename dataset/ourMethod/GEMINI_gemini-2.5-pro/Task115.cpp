#include <iostream>
#include <cstdint>
#include <iomanip>

// Define a struct with bit fields.
// The total number of bits is 1 + 1 + 2 + 3 = 7 bits.
// The compiler will likely pack this into a single byte (or an int, depending on implementation).
struct HardwareFlags {
    // LSB (Least Significant Bit) fields are often declared first
    // but the actual order is implementation-defined.
    uint8_t isActive : 1;
    uint8_t isReady : 1;
    uint8_t errorState : 2; // Can hold values 0-3
    uint8_t mode : 3;       // Can hold values 0-7
};

void printFlags(const HardwareFlags& flags, int testCaseNum) {
    std::cout << "Test Case " << testCaseNum << ": "
              << "isActive=" << static_cast<int>(flags.isActive)
              << ", isReady=" << static_cast<int>(flags.isReady)
              << ", errorState=" << static_cast<int>(flags.errorState)
              << ", mode=" << static_cast<int>(flags.mode)
              << std::endl;
}

int main() {
    std::cout << "--- C++ Bit Field Test Cases ---" << std::endl;
    std::cout << "Size of HardwareFlags struct: " << sizeof(HardwareFlags) << " byte(s)" << std::endl;

    // Test Case 1: All fields zero-initialized
    HardwareFlags flags1 = {0, 0, 0, 0};
    printFlags(flags1, 1);

    // Test Case 2: Setting individual fields
    HardwareFlags flags2 = {1, 0, 0, 5};
    printFlags(flags2, 2);

    // Test Case 3: Setting all fields to max values
    HardwareFlags flags3 = {1, 1, 3, 7};
    printFlags(flags3, 3);

    // Test Case 4: Setting and then changing fields
    HardwareFlags flags4 = {0, 1, 1, 4};
    std::cout << "Test Case 4 (before): "
              << "isActive=" << static_cast<int>(flags4.isActive)
              << ", isReady=" << static_cast<int>(flags4.isReady)
              << ", errorState=" << static_cast<int>(flags4.errorState)
              << ", mode=" << static_cast<int>(flags4.mode)
              << std::endl;
    flags4.isReady = 0;
    flags4.errorState = 2;
    std::cout << "Test Case 4 (after):  "
              << "isActive=" << static_cast<int>(flags4.isActive)
              << ", isReady=" << static_cast<int>(flags4.isReady)
              << ", errorState=" << static_cast<int>(flags4.errorState)
              << ", mode=" << static_cast<int>(flags4.mode)
              << std::endl;

    // Test Case 5: Partial initialization
    HardwareFlags flags5 = {1, 1, 2, 0};
    printFlags(flags5, 5);
    
    return 0;
}