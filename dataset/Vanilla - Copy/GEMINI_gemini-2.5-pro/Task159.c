#include <stdio.h>
#include <stdlib.h>

/**
 * Reads an integer value from a dynamically allocated buffer at a given index.
 *
 * @param buffer The pointer to the integer buffer.
 * @param size   The size of the buffer.
 * @param index  The index from which to read the data.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int readFromBuffer(const int* buffer, int size, int index) {
    if (buffer == NULL || index < 0 || index >= size) {
        fprintf(stderr, "Error: Index %d is out of bounds.\n", index);
        return -1; // Return a sentinel value for an invalid index
    }
    return buffer[index];
}

void printBuffer(const int* buffer, int size) {
    printf("Buffer created: [");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", buffer[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}


int main() {
    int bufferSize = 10;
    int* dynamicBuffer = NULL;

    // 1. Dynamically allocate a memory buffer
    dynamicBuffer = (int*)malloc(bufferSize * sizeof(int));

    // Check if allocation was successful
    if (dynamicBuffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1; // Exit with an error code
    }

    // 2. Initialize the buffer with some data
    for (int i = 0; i < bufferSize; ++i) {
        dynamicBuffer[i] = i * 10;
    }
    printBuffer(dynamicBuffer, bufferSize);
    printf("------------------------------------\n");

    // 3. Test cases to read data from the buffer
    // Test Case 1: Read from a valid index at the beginning
    int index1 = 0;
    int value1 = readFromBuffer(dynamicBuffer, bufferSize, index1);
    printf("Reading from index %d: %d\n", index1, value1);

    // Test Case 2: Read from a valid index in the middle
    int index2 = 5;
    int value2 = readFromBuffer(dynamicBuffer, bufferSize, index2);
    printf("Reading from index %d: %d\n", index2, value2);

    // Test Case 3: Read from a valid index at the end
    int index3 = bufferSize - 1;
    int value3 = readFromBuffer(dynamicBuffer, bufferSize, index3);
    printf("Reading from index %d: %d\n", index3, value3);

    // Test Case 4: Read from an invalid index (negative)
    int index4 = -1;
    int value4 = readFromBuffer(dynamicBuffer, bufferSize, index4);
    printf("Reading from index %d: %d\n", index4, value4);

    // Test Case 5: Read from an invalid index (out of bounds)
    int index5 = bufferSize;
    int value5 = readFromBuffer(dynamicBuffer, bufferSize, index5);
    printf("Reading from index %d: %d\n", index5, value5);

    // 4. Deallocate the memory to prevent memory leaks
    free(dynamicBuffer);
    dynamicBuffer = NULL; // Good practice to nullify pointer after freeing

    return 0;
}