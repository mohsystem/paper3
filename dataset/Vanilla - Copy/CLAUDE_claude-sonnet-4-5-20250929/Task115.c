
#include <stdio.h>

struct BitFields {
    unsigned int field1 : 3;  // 3 bits
    unsigned int field2 : 5;  // 5 bits
    unsigned int field3 : 8;  // 8 bits
};

void display(struct BitFields* bf) {
    printf("Field1: %u, Field2: %u, Field3: %u\\n", 
           bf->field1, bf->field2, bf->field3);
}

int main() {
    // Test case 1
    struct BitFields bf1;
    bf1.field1 = 5;
    bf1.field2 = 20;
    bf1.field3 = 100;
    printf("Test 1: ");
    display(&bf1);
    
    // Test case 2
    struct BitFields bf2;
    bf2.field1 = 7;
    bf2.field2 = 31;
    bf2.field3 = 255;
    printf("Test 2: ");
    display(&bf2);
    
    // Test case 3
    struct BitFields bf3;
    bf3.field1 = 0;
    bf3.field2 = 0;
    bf3.field3 = 0;
    printf("Test 3: ");
    display(&bf3);
    
    // Test case 4
    struct BitFields bf4;
    bf4.field1 = 3;
    bf4.field2 = 15;
    bf4.field3 = 128;
    printf("Test 4: ");
    display(&bf4);
    
    // Test case 5
    struct BitFields bf5;
    bf5.field1 = 1;
    bf5.field2 = 10;
    bf5.field3 = 50;
    printf("Test 5: ");
    display(&bf5);
    
    return 0;
}
