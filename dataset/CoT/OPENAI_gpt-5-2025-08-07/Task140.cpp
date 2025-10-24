// Chain-of-Through process:
// 1) Problem understanding: Function to perform arithmetic (+, -, *, /) based on operator char.
// 2) Security requirements: Avoid unsafe input; handle division by zero; invalid operators return NaN.
// 3) Secure coding generation: Use switch and return std::numeric_limits<double>::quiet_NaN() for errors.
// 4) Code review: Check all branches handled and no UB.
// 5) Secure code output: Include 5 deterministic test cases.

#include <iostream>
#include <limits>

double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b == 0.0) ? std::numeric_limits<double>::quiet_NaN() : (a / b);
        default:  return std::numeric_limits<double>::quiet_NaN();
    }
}

int main() {
    // 5 test cases
    struct Test { double a; double b; char op; } tests[5] = {
        {10.0, 5.0, '+'},
        {10.0, 5.0, '-'},
        {10.0, 5.0, '*'},
        {10.0, 0.0, '/'},
        {10.0, 5.0, '%'} // invalid operator
    };

    for (int i = 0; i < 5; ++i) {
        double result = calculate(tests[i].a, tests[i].b, tests[i].op);
        std::cout << "Test " << (i + 1) << ": " << tests[i].a << " " << tests[i].op << " " << tests[i].b
                  << " = " << result << "\n";
    }
    return 0;
}