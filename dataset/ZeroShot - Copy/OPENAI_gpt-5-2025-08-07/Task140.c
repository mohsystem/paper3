#include <stdio.h>

typedef struct {
    int ok;        /* 1 on success, 0 on error */
    double value;  /* valid when ok == 1 */
    const char* error; /* non-NULL on error */
} CalcResult;

CalcResult calculate(double a, double b, char op) {
    CalcResult res;
    res.ok = 0;
    res.value = 0.0;
    res.error = "Error: Unsupported operator. Use one of +, -, *, /.";

    switch (op) {
        case '+':
            res.ok = 1;
            res.value = a + b;
            res.error = NULL;
            break;
        case '-':
            res.ok = 1;
            res.value = a - b;
            res.error = NULL;
            break;
        case '*':
            res.ok = 1;
            res.value = a * b;
            res.error = NULL;
            break;
        case '/':
            if (b == 0.0) {
                res.ok = 0;
                res.error = "Error: Division by zero.";
            } else {
                res.ok = 1;
                res.value = a / b;
                res.error = NULL;
            }
            break;
        default:
            /* already set to unsupported operator */
            break;
    }
    return res;
}

int main(void) {
    struct Test { double a; double b; char op; };
    struct Test tests[5] = {
        {10.0, 5.0, '+'},
        {10.0, 5.0, '-'},
        {10.0, 5.0, '*'},
        {10.0, 5.0, '/'},
        {10.0, 0.0, '/'}
    };

    for (int i = 0; i < 5; i++) {
        CalcResult res = calculate(tests[i].a, tests[i].b, tests[i].op);
        if (res.ok) {
            printf("%.6f %c %.6f = %.6f\n", tests[i].a, tests[i].op, tests[i].b, res.value);
        } else {
            printf("Operation %.6f %c %.6f failed: %s\n", tests[i].a, tests[i].op, tests[i].b, res.error);
        }
    }
    return 0;
}