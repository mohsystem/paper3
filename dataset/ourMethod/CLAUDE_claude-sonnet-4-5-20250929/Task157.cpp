
#include <iostream>
#include <memory>
#include <cstring>
#include <limits>
#include <stdexcept>

// Maximum allocation size to prevent excessive memory usage
constexpr size_t MAX_ALLOCATION_SIZE = 1024 * 1024 * 100; // 100 MB

// Function to dynamically allocate and deallocate memory
// Returns true on success, false on failure
bool allocateAndDeallocate(size_t size) {
    // Input validation: check size is within safe bounds
    if (size == 0) {
        std::cerr << "Error: Allocation size must be greater than 0" << std::endl;
        return false;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        std::cerr << "Error: Allocation size exceeds maximum allowed" << std::endl;
        return false;
    }
    
    try {
        // Use smart pointer for automatic memory management (RAII)
        // This ensures memory is deallocated even if exceptions occur
        std::unique_ptr<unsigned char[]> buffer(new unsigned char[size]);
        
        if (!buffer) {
            std::cerr << "Error: Memory allocation failed" << std::endl;
            return false;
        }
        
        // Initialize allocated memory to prevent use of uninitialized data
        std::memset(buffer.get(), 0, size);
        
        // Perform some operation to demonstrate usage
        buffer[0] = 0xAA;
        if (size > 1) {
            buffer[size - 1] = 0xBB;
        }
        
        std::cout << "Successfully allocated " << size << " bytes" << std::endl;
        std::cout << "First byte: 0x" << std::hex << static_cast<int>(buffer[0]) << std::endl;
        if (size > 1) {
            std::cout << "Last byte: 0x" << std::hex << static_cast<int>(buffer[size - 1]) << std::endl;
        }
        std::cout << std::dec; // Reset to decimal output
        
        // Clear sensitive data before deallocation (defense in depth)
        std::memset(buffer.get(), 0, size);
        
        // Memory automatically deallocated when unique_ptr goes out of scope
        std::cout << "Memory deallocated successfully" << std::endl;
        return true;
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Memory allocation failed - " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception occurred - " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "=== Memory Allocation Test Cases ===" << std::endl;
    
    // Test case 1: Small allocation
    std::cout << "\\nTest 1: Allocate 64 bytes" << std::endl;
    allocateAndDeallocate(64);
    
    // Test case 2: Medium allocation
    std::cout << "\\nTest 2: Allocate 1024 bytes" << std::endl;
    allocateAndDeallocate(1024);
    
    // Test case 3: Large allocation
    std::cout << "\\nTest 3: Allocate 1 MB" << std::endl;
    allocateAndDeallocate(1024 * 1024);
    
    // Test case 4: Invalid - zero size
    std::cout << "\\nTest 4: Allocate 0 bytes (should fail)" << std::endl;
    allocateAndDeallocate(0);
    
    // Test case 5: Invalid - exceeds maximum
    std::cout << "\\nTest 5: Allocate excessive size (should fail)" << std::endl;
    allocateAndDeallocate(MAX_ALLOCATION_SIZE + 1);
    
    return 0;
}
