
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void copyBytes(unsigned char* destination, const unsigned char* source, size_t numBytes) {
    if (destination == NULL || source == NULL) {
        fprintf(stderr, "Error: Destination and source cannot be null\\n");
        return;
    }
    
    if (numBytes == 0) {
        return;
    }
    
    memcpy(destination, source, numBytes);
}

int main() {
    // Test case 1: Normal copy
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dst1[5] = {0};
    copyBytes(dst1, src1, 5);
    printf("Test 1: ");
    for (int i = 0; i < 5; i++) printf("%d ", dst1[i]);
    printf("\\n");
    
    // Test case 2: Partial copy
    unsigned char src2[] = {10, 20, 30, 40, 50};
    unsigned char dst2[5] = {0};
    copyBytes(dst2, src2, 3);
    printf("Test 2: ");
    for (int i = 0; i < 5; i++) printf("%d ", dst2[i]);
    printf("\\n");
    
    // Test case 3: Copy 0 bytes
    unsigned char src3[] = {1, 2, 3};
    unsigned char dst3[3] = {0};
    copyBytes(dst3, src3, 0);
    printf("Test 3: ");
    for (int i = 0; i < 3; i++) printf("%d ", dst3[i]);
    printf("\\n");
    
    // Test case 4: Single byte copy
    unsigned char src4[] = {99};
    unsigned char dst4[1] = {0};
    copyBytes(dst4, src4, 1);
    printf("Test 4: ");
    for (int i = 0; i < 1; i++) printf("%d ", dst4[i]);
    printf("\\n");
    
    // Test case 5: Larger array
    unsigned char src5[] = {11, 22, 33, 44, 55, 66, 77, 88};
    unsigned char dst5[8] = {0};
    copyBytes(dst5, src5, 8);
    printf("Test 5: ");
    for (int i = 0; i < 8; i++) printf("%d ", dst5[i]);
    printf("\\n");
    
    return 0;
}
