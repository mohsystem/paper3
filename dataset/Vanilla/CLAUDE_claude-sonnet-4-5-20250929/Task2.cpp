
#include <iostream>
#include <string>
using namespace std;

string createPhoneNumber(int numbers[]) {
    char result[15];
    sprintf(result, "(%d%d%d) %d%d%d-%d%d%d%d",
        numbers[0], numbers[1], numbers[2],
        numbers[3], numbers[4], numbers[5],
        numbers[6], numbers[7], numbers[8], numbers[9]);
    return string(result);
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    cout << createPhoneNumber(test1) << endl;
    
    // Test case 2
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    cout << createPhoneNumber(test2) << endl;
    
    // Test case 3
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    cout << createPhoneNumber(test3) << endl;
    
    // Test case 4
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    cout << createPhoneNumber(test4) << endl;
    
    // Test case 5
    int test5[] = {8, 6, 7, 5, 3, 0, 9, 1, 2, 3};
    cout << createPhoneNumber(test5) << endl;
    
    return 0;
}
