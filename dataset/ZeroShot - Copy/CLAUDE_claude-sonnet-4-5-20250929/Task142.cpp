
#include <iostream>
#include <cstdlib>
using namespace std;

int findGCD(int a, int b) {
    a = abs(a);
    b = abs(b);
    
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    // Test case 1
    cout << "GCD of 48 and 18: " << findGCD(48, 18) << endl;
    
    // Test case 2
    cout << "GCD of 100 and 50: " << findGCD(100, 50) << endl;
    
    // Test case 3
    cout << "GCD of 17 and 19: " << findGCD(17, 19) << endl;
    
    // Test case 4
    cout << "GCD of 270 and 192: " << findGCD(270, 192) << endl;
    
    // Test case 5
    cout << "GCD of 1071 and 462: " << findGCD(1071, 462) << endl;
    
    return 0;
}
