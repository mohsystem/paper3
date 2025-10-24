/* Chain-of-Through process:
   1) Problem understanding: Implement arithmetic based on operator (+, -, *, /).
   2) Security requirements: No unsafe input; handle divide-by-zero; invalid operator returns NaN.
   3) Secure coding generation: Use switch; return NAN on invalid or unsafe operations.
   4) Code review: Ensure no undefined behavior; handle all cases.
   5) Secure code output: Provide 5 test cases. */

#include <stdio.h>
#include <math.h>

double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b == 0.0) ? NAN : (a / b);
        default:  return NAN;
    }
}

int main(void) {
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
        printf("Test %d: %.6f %c %.6f = %f\n", i + 1, tests[i].a, tests[i].op, tests[i].b, result);
    }
    return 0;
}