
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

// Security: Define constants for buffer and read sizes
const size_t BUFFER_SIZE = 1024;
const size_t READ_SIZE = 10;

// Allocates a buffer of 1024 bytes
// Security: Returns pointer to initialized memory, caller must free
// Returns: pointer to allocated buffer or NULL on failure
unsigned char* allocateBuffer() {
    // Security: Use calloc to zero-initialize, preventing information leakage
    unsigned char* buf = static_cast<unsigned char*>(calloc(BUFFER_SIZE, sizeof(unsigned char)));
    
    // Security: Check allocation result
    if (buf == nullptr) {
        throw std::bad_alloc();
    }
    
    return buf;
}

// Reads exactly READ_SIZE characters from stdin and stores in buffer
// Security: Validates buffer, performs bounds checking, limits read size
// Returns: number of bytes actually read and stored
size_t readCharactersToBuffer(unsigned char* buf) {
    // Security: Validate buffer pointer
    if (buf == nullptr) {
        throw std::invalid_argument("Buffer cannot be NULL");
    }
    
    // Security: Clear buffer before use to ensure clean state
    memset(buf, 0, BUFFER_SIZE);
    
    // Security: Read exactly READ_SIZE bytes, no more
    // fread is safer than gets/scanf for fixed-size reads
    size_t bytesRead = fread(buf, sizeof(unsigned char), READ_SIZE, stdin);
    
    // Security: Ensure null termination if treating as string
    // This is safe because BUFFER_SIZE > READ_SIZE
    if (bytesRead < BUFFER_SIZE) {
        buf[bytesRead] = '\\0';
    }
    
    // Security: Check for read errors (but not EOF which is valid)
    if (ferror(stdin)) {
        clearerr(stdin);
        throw std::runtime_error("Error reading input");
    }
    
    return bytesRead;
}

// Security: Helper to securely free and clear sensitive buffer
void secureFreeBuffer(unsigned char* buf) {
    if (buf != nullptr) {
        // Security: Zero memory before freeing to prevent data leakage
        memset(buf, 0, BUFFER_SIZE);
        free(buf);
    }
}

int main() {
    unsigned char* buf = nullptr;
    
    try {
        // Test case 1: Normal read
        std::cerr << "Test 1: Enter 10 characters:" << std::endl;
        buf = allocateBuffer();
        size_t read1 = readCharactersToBuffer(buf);
        std::cerr << "Bytes read: " << read1 << std::endl;
        secureFreeBuffer(buf);
        buf = nullptr;
        
        // Test case 2: Another normal read
        std::cerr << "\\nTest 2: Enter 10 characters:" << std::endl;
        buf = allocateBuffer();
        size_t read2 = readCharactersToBuffer(buf);
        std::cerr << "Bytes read: " << read2 << std::endl;
        secureFreeBuffer(buf);
        buf = nullptr;
        
        // Test case 3: Short input
        std::cerr << "\\nTest 3: Enter less than 10 characters:" << std::endl;
        buf = allocateBuffer();
        size_t read3 = readCharactersToBuffer(buf);
        std::cerr << "Bytes read: " << read3 << std::endl;
        secureFreeBuffer(buf);
        buf = nullptr;
        
        // Test case 4: Binary data
        std::cerr << "\\nTest 4: Enter 10 characters:" << std::endl;
        buf = allocateBuffer();
        size_t read4 = readCharactersToBuffer(buf);
        std::cerr << "Bytes read: " << read4 << std::endl;
        secureFreeBuffer(buf);
        buf = nullptr;
        
        // Test case 5: Another read
        std::cerr << "\\nTest 5: Enter 10 characters:" << std::endl;
        buf = allocateBuffer();
        size_t read5 = readCharactersToBuffer(buf);
        std::cerr << "Bytes read: " << read5 << std::endl;
        secureFreeBuffer(buf);
        buf = nullptr;
        
    } catch (const std::exception& e) {
        // Security: Generic error message
        std::cerr << "Error: " << e.what() << std::endl;
        if (buf != nullptr) {
            secureFreeBuffer(buf);
        }
        return 1;
    }
    
    return 0;
}
