
#include <stdio.h>
#include <string.h>

void copyBytes(unsigned char* destination, const unsigned char* source, int numBytes) {
    if (destination == NULL || source == NULL || numBytes <= 0) {
        return;
    }
    
    for (int i = 0; i < numBytes; i++) {
        destination[i] = source[i];
    }
}

int main() {
    // Test case 1: Normal copy
    unsigned char dest1[10] = {0};
    unsigned char src1[] = {1, 2, 3, 4, 5};
    copyBytes(dest1, src1, 5);
    printf("Test 1: ");
    for (int i = 0; i < 5; i++) printf("%d ", dest1[i]);
    printf("\\n");
    
    // Test case 2: Copy fewer bytes than source
    unsigned char dest2[10] = {0};
    unsigned char src2[] = {10, 20, 30, 40, 50};
    copyBytes(dest2, src2, 3);
    printf("Test 2: ");
    for (int i = 0; i < 3; i++) printf("%d ", dest2[i]);
    printf("\\n");
    
    // Test case 3: Copy with larger numBytes than source
    unsigned char dest3[10] = {0};
    unsigned char src3[] = {7, 8, 9};
    copyBytes(dest3, src3, 3);
    printf("Test 3: ");
    for (int i = 0; i < 3; i++) printf("%d ", dest3[i]);
    printf("\\n");
    
    // Test case 4: Copy zero bytes
    unsigned char dest4[5] = {0};
    unsigned char src4[] = {11, 12, 13};
    copyBytes(dest4, src4, 0);
    printf("Test 4: ");
    for (int i = 0; i < 5; i++) printf("%d ", dest4[i]);
    printf("\\n");
    
    // Test case 5: Copy all bytes
    unsigned char dest5[5] = {0};
    unsigned char src5[] = {100, 101, 102, 103, 104};
    copyBytes(dest5, src5, 5);
    printf("Test 5: ");
    for (int i = 0; i < 5; i++) printf("%d ", dest5[i]);
    printf("\\n");
    
    return 0;
}
