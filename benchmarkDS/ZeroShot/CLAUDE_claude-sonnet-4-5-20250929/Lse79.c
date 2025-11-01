
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* buffer;
    size_t size;
} MemoryBuffer;

MemoryBuffer* createBuffer(size_t size) {
    MemoryBuffer* mb = (MemoryBuffer*)malloc(sizeof(MemoryBuffer));
    if (mb == NULL) {
        return NULL;
    }
    
    mb->size = size;
    mb->buffer = (char*)malloc(size);
    if (mb->buffer == NULL) {
        free(mb);
        return NULL;
    }
    
    memset(mb->buffer, 0, size);
    return mb;
}

void destroyBuffer(MemoryBuffer* mb) {
    if (mb != NULL) {
        if (mb->buffer != NULL) {
            free(mb->buffer);
        }
        free(mb);
    }
}

void readInput(MemoryBuffer* mb, size_t maxChars) {
    if (mb == NULL || mb->buffer == NULL) {
        return;
    }
    
    size_t charsToRead = (maxChars < mb->size - 1) ? maxChars : mb->size - 1;
    
    if (fgets(mb->buffer, charsToRead + 1, stdin) != NULL) {
        size_t len = strlen(mb->buffer);
        if (len > 0 && mb->buffer[len - 1] == '\\n') {
            mb->buffer[len - 1] = '\\0';
        }
    }
}

void allocateAndReadBuffers() {
    MemoryBuffer* buf = createBuffer(1024);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate buffer 1\\n");
        return;
    }
    
    readInput(buf, 100);
    
    MemoryBuffer* buf1 = createBuffer(1024);
    if (buf1 == NULL) {
        fprintf(stderr, "Failed to allocate buffer 2\\n");
        destroyBuffer(buf);
        return;
    }
    
    readInput(buf1, 100);
    
    printf("Buffer 1: %s\\n", buf->buffer);
    printf("Buffer 2: %s\\n", buf1->buffer);
    
    destroyBuffer(buf);
    destroyBuffer(buf1);
}

int main() {
    printf("Test cases for memory allocation and input reading:\\n");
    
    printf("\\nTest Case 1: Normal execution\\n");
    allocateAndReadBuffers();
    
    return 0;
}
