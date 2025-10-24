
#include <iostream>
#include <cmath>
using namespace std;

double calculate(double num1, double num2, char op) {
    switch (op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if (num2 == 0) {
                cout << "Error: Division by zero" << endl;
                return NAN;
            }
            return num1 / num2;
        default:
            cout << "Error: Invalid operator" << endl;
            return NAN;
    }
}

int main() {
    // Test case 1: Addition
    cout << "Test 1: 10 + 5 = " << calculate(10, 5, '+') << endl;
    
    // Test case 2: Subtraction
    cout << "Test 2: 20 - 8 = " << calculate(20, 8, '-') << endl;
    
    // Test case 3: Multiplication
    cout << "Test 3: 7 * 6 = " << calculate(7, 6, '*') << endl;
    
    // Test case 4: Division
    cout << "Test 4: 50 / 2 = " << calculate(50, 2, '/') << endl;
    
    // Test case 5: Division by zero
    cout << "Test 5: 10 / 0 = " << calculate(10, 0, '/') << endl;
    
    return 0;
}
