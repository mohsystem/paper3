
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Secure memory zeroization - prevents compiler optimization */
static void secure_zero(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) {
        return;
    }
    volatile unsigned char* vptr = (volatile unsigned char*)ptr;
    for (size_t i = 0; i < size; i++) {
        vptr[i] = 0;
    }
}

/* Function to allocate, copy, and free memory chunks */
void allocateCopyAndFree(void) {
    unsigned char* chunk1 = NULL;
    unsigned char* chunk2 = NULL;
    const size_t CHUNK_SIZE = 10;
    
    /* Allocate first chunk of 10 bytes with NULL check */
    chunk1 = (unsigned char*)malloc(CHUNK_SIZE);
    if (chunk1 == NULL) {
        fprintf(stderr, "Failed to allocate chunk1\\n");
        return;
    }
    
    /* Initialize chunk1 to zero for safety */
    memset(chunk1, 0, CHUNK_SIZE);
    
    /* Fill chunk1 with sample data for testing */
    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        chunk1[i] = (unsigned char)i;
    }
    
    /* Allocate second chunk of 10 bytes with NULL check */
    chunk2 = (unsigned char*)malloc(CHUNK_SIZE);
    if (chunk2 == NULL) {
        fprintf(stderr, "Failed to allocate chunk2\\n");
        /* Clean up chunk1 before returning */
        secure_zero(chunk1, CHUNK_SIZE);
        free(chunk1);
        return;
    }
    
    /* Initialize chunk2 to zero for safety */
    memset(chunk2, 0, CHUNK_SIZE);
    
    /* Copy contents from chunk1 to chunk2 - memcpy safe as no overlap */
    memcpy(chunk2, chunk1, CHUNK_SIZE);
    
    /* Clear sensitive data from chunk1 before freeing */
    secure_zero(chunk1, CHUNK_SIZE);
    free(chunk1);
    chunk1 = NULL;
    
    /* Clear sensitive data from chunk2 before freeing */
    secure_zero(chunk2, CHUNK_SIZE);
    free(chunk2);
    chunk2 = NULL;
}

int main(void) {
    /* Test case 1: Normal operation */
    printf("Test 1: Normal allocation, copy, and free\\n");
    allocateCopyAndFree();
    
    /* Test case 2: Repeated operations */
    printf("Test 2: Repeated operations\\n");
    allocateCopyAndFree();
    
    /* Test case 3: Multiple sequential calls */
    printf("Test 3: Multiple sequential calls\\n");
    for (int i = 0; i < 3; i++) {
        allocateCopyAndFree();
    }
    
    /* Test case 4: Single operation */
    printf("Test 4: Single operation\\n");
    allocateCopyAndFree();
    
    /* Test case 5: Final operation */
    printf("Test 5: Final operation\\n");
    allocateCopyAndFree();
    
    printf("All tests completed successfully\\n");
    
    return 0;
}
