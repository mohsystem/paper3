
#include <iostream>
#include <cstdlib>
using namespace std;

int gcd(int a, int b) {
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
    cout << "GCD of 48 and 18: " << gcd(48, 18) << endl;
    
    // Test case 2
    cout << "GCD of 100 and 50: " << gcd(100, 50) << endl;
    
    // Test case 3
    cout << "GCD of 17 and 19: " << gcd(17, 19) << endl;
    
    // Test case 4
    cout << "GCD of 270 and 192: " << gcd(270, 192) << endl;
    
    // Test case 5
    cout << "GCD of 1071 and 462: " << gcd(1071, 462) << endl;
    
    return 0;
}
