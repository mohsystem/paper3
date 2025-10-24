#include <iostream>
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
                return {false, 0.0, "Error: Division by zero."};
            }
            return {true, a / b, ""};
        default:
            return {false, 0.0, "Error: Unsupported operator. Use one of +, -, *, /."};
    }
}

int main() {
    struct Test { double a; double b; char op; };
    Test tests[5] = {
        {10.0, 5.0, '+'},
        {10.0, 5.0, '-'},
        {10.0, 5.0, '*'},
        {10.0, 5.0, '/'},
        {10.0, 0.0, '/'}
    };

    for (const auto& t : tests) {
        CalcResult res = calculate(t.a, t.b, t.op);
        if (res.ok) {
            std::cout << t.a << " " << t.op << " " << t.b << " = " << res.value << "\n";
        } else {
            std::cout << "Operation " << t.a << " " << t.op << " " << t.b << " failed: " << res.error << "\n";
        }
    }
    return 0;
}