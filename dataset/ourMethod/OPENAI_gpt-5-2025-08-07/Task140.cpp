#include <iostream>
#include <iomanip>
#include <string>

struct CalcResult {
    bool ok;
    double value;
    std::string error;
};

CalcResult calculate(double a, double b, char op) {
    switch (op) {
        case '+':
            return {true, a + b, ""};
        case '-':
            return {true, a - b, ""};
        case '*':
            return {true, a * b, ""};
        case '/':
            if (b == 0.0) {
                return {false, 0.0, "Division by zero"};
            }
            return {true, a / b, ""};
        default:
            return {false, 0.0, "Invalid operator"};
    }
}

static void printResult(double a, double b, char op, const CalcResult& res) {
    std::cout << std::fixed << std::setprecision(6);
    if (res.ok) {
        std::cout << "Input: " << a << " " << op << " " << b << " => Result: " << res.value << "\n";
    } else {
        std::cout << "Input: " << a << " " << op << " " << b << " => Error: " << res.error << "\n";
    }
}

int main() {
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