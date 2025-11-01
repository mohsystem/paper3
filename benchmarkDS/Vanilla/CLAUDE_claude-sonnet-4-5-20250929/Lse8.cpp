
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

std::string getValueAtIndex(char* args[], int argc, int index) {
    if (args == nullptr || index < 0 || index >= argc) {
        return "";
    }
    return std::string(args[index]);
}

int main() {
    // Test case 1
    char* test1[] = {(char*)"program", (char*)"1", (char*)"hello", (char*)"world"};
    int argc1 = 4;
    int index1 = std::atoi(test1[1]);
    std::cout << "Test 1: " << getValueAtIndex(test1, argc1, index1) << std::endl;
    
    // Test case 2
    char* test2[] = {(char*)"program", (char*)"2", (char*)"apple", (char*)"banana", (char*)"cherry"};
    int argc2 = 5;
    int index2 = std::atoi(test2[1]);
    std::cout << "Test 2: " << getValueAtIndex(test2, argc2, index2) << std::endl;
    
    // Test case 3
    char* test3[] = {(char*)"program", (char*)"0", (char*)"first"};
    int argc3 = 3;
    int index3 = std::atoi(test3[1]);
    std::cout << "Test 3: " << getValueAtIndex(test3, argc3, index3) << std::endl;
    
    // Test case 4
    char* test4[] = {(char*)"program", (char*)"3", (char*)"a", (char*)"b", (char*)"c", (char*)"d"};
    int argc4 = 6;
    int index4 = std::atoi(test4[1]);
    std::cout << "Test 4: " << getValueAtIndex(test4, argc4, index4) << std::endl;
    
    // Test case 5
    char* test5[] = {(char*)"program", (char*)"1", (char*)"test"};
    int argc5 = 3;
    int index5 = std::atoi(test5[1]);
    std::cout << "Test 5: " << getValueAtIndex(test5, argc5, index5) << std::endl;
    
    return 0;
}
