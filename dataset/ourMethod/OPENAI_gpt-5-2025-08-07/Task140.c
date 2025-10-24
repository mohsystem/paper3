#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    bool ok;
    double value;
    char error[64];
} CalcResult;

CalcResult calculate(double a, double b, char op) {
    CalcResult res;
    res.ok = false;
    res.value = 0.0;
    res.error[0] = '\0';

    switch (op) {
        case '+':
            res.ok = true;
            res.value = a + b;
            break;
        case '-':
            res.ok = true;
            res.value = a - b;
            break;
        case '*':
            res.ok = true;
            res.value = a * b;
            break;
        case '/':
            if (b == 0.0) {
                snprintf(res.error, sizeof(res.error), "%s", "Division by zero");
                res.ok = false;
            } else {
                res.ok = true;
                res.value = a / b;
            }
            break;
        default:
            snprintf(res.error, sizeof(res.error), "%s", "Invalid operator");
            res.ok = false;
            break;
    }
    return res;
}

static void printResult(double a, double b, char op, CalcResult res) {
    if (res.ok) {
        printf("Input: %.6f %c %.6f => Result: %.6f\n", a, op, b, res.value);
    } else {
        printf("Input: %.6f %c %.6f => Error: %s\n", a, op, b, res.error);
    }
}

int main(void) {
    CalcResult r1 = calculate(3.0, 5.0, '+');
    CalcResult r2 = calculate(10.0, 3.0, '-');
    CalcResult r3 = calculate(-2.5, 4.0, '*');
    CalcResult r4 = calculate(7.0, 2.0, '/');
    CalcResult r5 = calculate(5.0, 0.0, '/');

    printResult(3.0, 5.0, '+', r1);
    printResult(10.0, 3.0, '-', r2);
    printResult(-2.5, 4.0, '*', r3);
    printResult(7.0, 2.0, '/', r4);
    printResult(5.0, 0.0, '/', r5);

    return 0;
}