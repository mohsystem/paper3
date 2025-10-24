
#include <iostream>
#include <string>
#include <cstring>
#include <limits>

// Securely handles user input into a fixed-size buffer
// Returns true on success, false on failure
bool handleUserInput(char* buffer, size_t bufferSize) {
    // Validate input parameters - treat all inputs as untrusted
    if (buffer == nullptr || bufferSize == 0) {
        std::cerr << "Error: Invalid buffer parameters" << std::endl;
        return false;
    }
    
    // Define a reasonable maximum input size to prevent excessive memory usage
    const size_t MAX_INPUT_SIZE = 4096;
    if (bufferSize > MAX_INPUT_SIZE) {
        std::cerr << "Error: Buffer size exceeds maximum allowed" << std::endl;
        return false;
    }
    
    // Initialize buffer to zero - prevent information leakage
    std::memset(buffer, 0, bufferSize);
    
    // Read input safely using std::cin.getline with explicit buffer size
    // This prevents buffer overflow by enforcing boundary checks
    std::cin.getline(buffer, bufferSize);
    
    // Check if input was truncated or if error occurred
    if (std::cin.fail() && !std::cin.eof()) {
        if (std::cin.gcount() == static_cast<std::streamsize>(bufferSize - 1)) {
            std::cerr << "Warning: Input truncated to fit buffer size" << std::endl;
        }
        // Clear error flags and discard remaining input
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
    }
    
    // Ensure null termination - defensive programming
    buffer[bufferSize - 1] = '\\0';
    
    // Validate that input contains only printable ASCII characters
    // This prevents injection of control characters
    size_t len = std::strlen(buffer);
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = static_cast<unsigned char>(buffer[i]);
        if (c < 32 || c > 126) {
            std::cerr << "Error: Input contains invalid characters" << std::endl;
            // Clear buffer on validation failure
            std::memset(buffer, 0, bufferSize);
            return false;
        }
    }
    
    return true;
}

int main() {
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    
    std::cout << "=== User Input Buffer Handler Test Cases ===" << std::endl;
    
    // Test case 1: Normal input
    std::cout << "\\nTest 1 - Enter normal text (e.g., 'Hello World'): ";
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        std::cout << "Success: Input stored: '" << buffer << "'" << std::endl;
    } else {
        std::cout << "Failed to handle input" << std::endl;
    }
    
    // Test case 2: Empty input
    std::cout << "\\nTest 2 - Press Enter for empty input: ";
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        std::cout << "Success: Empty input handled, length: " 
                  << std::strlen(buffer) << std::endl;
    } else {
        std::cout << "Failed to handle input" << std::endl;
    }
    
    // Test case 3: Maximum length input
    std::cout << "\\nTest 3 - Enter text close to buffer size (try 250+ chars): ";
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        std::cout << "Success: Input length: " << std::strlen(buffer) << std::endl;
    } else {
        std::cout << "Failed to handle input" << std::endl;
    }
    
    // Test case 4: Input with numbers and symbols
    std::cout << "\\nTest 4 - Enter text with numbers/symbols (e.g., 'Test123!@#'): ";
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        std::cout << "Success: Input stored: '" << buffer << "'" << std::endl;
    } else {
        std::cout << "Failed to handle input" << std::endl;
    }
    
    // Test case 5: Small buffer size
    std::cout << "\\nTest 5 - Enter short text (using 32-byte buffer): ";
    char smallBuffer[32];
    if (handleUserInput(smallBuffer, sizeof(smallBuffer))) {
        std::cout << "Success: Input stored: '" << smallBuffer << "'" << std::endl;
    } else {
        std::cout << "Failed to handle input" << std::endl;
    }
    
    return 0;
}
