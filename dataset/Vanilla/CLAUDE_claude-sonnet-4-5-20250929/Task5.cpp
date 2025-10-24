
#include <iostream>
using namespace std;

int persistence(int num) {
    int count = 0;
    
    while (num >= 10) {
        int product = 1;
        while (num > 0) {
            product *= num % 10;
            num /= 10;
        }
        num = product;
        count++;
    }
    
    return count;
}

int main() {
    // Test cases
    cout << "Input: 39, Output: " << persistence(39) << endl;
    cout << "Input: 999, Output: " << persistence(999) << endl;
    cout << "Input: 4, Output: " << persistence(4) << endl;
    cout << "Input: 25, Output: " << persistence(25) << endl;
    cout << "Input: 77, Output: " << persistence(77) << endl;
    
    return 0;
}
