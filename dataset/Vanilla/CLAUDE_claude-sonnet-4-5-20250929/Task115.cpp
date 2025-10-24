
#include <iostream>

struct BitFields {
    unsigned int field1 : 3;  // 3 bits
    unsigned int field2 : 5;  // 5 bits
    unsigned int field3 : 8;  // 8 bits
    
    void display() {
        std::cout << "Field1: " << field1 
                  << ", Field2: " << field2 
                  << ", Field3: " << field3 << std::endl;
    }
};

int main() {
    // Test case 1
    BitFields bf1;
    bf1.field1 = 5;
    bf1.field2 = 20;
    bf1.field3 = 100;
    std::cout << "Test 1: ";
    bf1.display();
    
    // Test case 2
    BitFields bf2;
    bf2.field1 = 7;
    bf2.field2 = 31;
    bf2.field3 = 255;
    std::cout << "Test 2: ";
    bf2.display();
    
    // Test case 3
    BitFields bf3;
    bf3.field1 = 0;
    bf3.field2 = 0;
    bf3.field3 = 0;
    std::cout << "Test 3: ";
    bf3.display();
    
    // Test case 4
    BitFields bf4;
    bf4.field1 = 3;
    bf4.field2 = 15;
    bf4.field3 = 128;
    std::cout << "Test 4: ";
    bf4.display();
    
    // Test case 5
    BitFields bf5;
    bf5.field1 = 1;
    bf5.field2 = 10;
    bf5.field3 = 50;
    std::cout << "Test 5: ";
    bf5.display();
    
    return 0;
}
