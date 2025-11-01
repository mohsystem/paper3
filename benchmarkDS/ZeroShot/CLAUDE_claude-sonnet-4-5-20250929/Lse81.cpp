
#include <iostream>
#include <cstdlib>
#include <cstring>

const int BUFFER_SIZE = 1024;
const int READ_SIZE = 10;

char* allocateBuffer() {
    char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (buffer != NULL) {
        memset(buffer, 0, BUFFER_SIZE);
    }
    return buffer;
}

int readCharacters(char* buf) {
    if (buf == NULL) {
        return -1;
    }
    
    int charsRead = 0;
    for (int i = 0; i < READ_SIZE && i < BUFFER_SIZE; i++) {
        int c = std::cin.get();
        if (c == EOF) {
            break;
        }
        buf[i] = (char)c;
        charsRead++;
    }
    
    return charsRead;
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    char* buffer1 = allocateBuffer();
    std::cout << "Buffer allocated with size: " << BUFFER_SIZE << std::endl;
    free(buffer1);
    
    std::cout << "\\nTest Case 2:" << std::endl;
    char* buffer2 = allocateBuffer();
    std::cout << "Enter 10 characters: " << std::endl;
    int read = readCharacters(buffer2);
    std::cout << "Characters read: " << read << std::endl;
    free(buffer2);
    
    std::cout << "\\nTest Case 3: Reading into buffer" << std::endl;
    char* buffer3 = allocateBuffer();
    for (int i = 0; i < READ_SIZE; i++) {
        buffer3[i] = 'A' + i;
    }
    std::cout << "Pre-filled buffer with test data" << std::endl;
    free(buffer3);
    
    std::cout << "\\nTest Case 4: Check buffer bounds" << std::endl;
    char* buffer4 = allocateBuffer();
    std::cout << "Buffer can safely hold " << READ_SIZE << " characters: " << (BUFFER_SIZE >= READ_SIZE) << std::endl;
    free(buffer4);
    
    std::cout << "\\nTest Case 5: Null buffer check" << std::endl;
    int result = readCharacters(NULL);
    std::cout << "Reading into null buffer returns: " << result << std::endl;
    
    return 0;
}
