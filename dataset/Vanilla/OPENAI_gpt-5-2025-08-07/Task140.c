#include <stdio.h>
#include <math.h>

double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? (a / b) : NAN;
        default: return NAN;
    }
}

int main() {
    printf("3 + 5 = %g\n", calculate(3, 5, '+'));
    printf("10 - 4 = %g\n", calculate(10, 4, '-'));
    printf("7 * 6 = %g\n", calculate(7, 6, '*'));
    printf("20 / 5 = %g\n", calculate(20, 5, '/'));
    printf("7 / 0 = %g\n", calculate(7, 0, '/'));
    return 0;
}