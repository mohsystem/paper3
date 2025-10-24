// Chain-of-Through process:
// 1) Problem understanding: Implement a function that takes two numbers and an operator (+, -, *, /) and returns the result.
// 2) Security requirements: Avoid unsafe inputs, handle division by zero, avoid exceptions for common invalid cases by returning NaN.
// 3) Secure coding generation: Use strict operator handling; no external input; handle invalid operators and division by zero by returning Double.NaN.
// 4) Code review: Ensure no unchecked operations; handle invalid operator cases; avoid resource use beyond basics.
// 5) Secure code output: Final code returns deterministic outputs and demonstrates 5 test cases.

public class Task140 {
    public static double calculate(double a, double b, char op) {
        switch (op) {
            case '+':
                return a + b;
            case '-':
                return a - b;
            case '*':
                return a * b;
            case '/':
                if (b == 0.0) return Double.NaN;
                return a / b;
            default:
                return Double.NaN; // Invalid operator
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        double[] aVals = {10.0, 10.0, 10.0, 10.0, 10.0};
        double[] bVals = {5.0, 5.0, 5.0, 0.0, 5.0};
        char[] ops    = {'+', '-', '*', '/', '%'}; // last one invalid

        for (int i = 0; i < aVals.length; i++) {
            double result = calculate(aVals[i], bVals[i], ops[i]);
            System.out.println("Test " + (i + 1) + ": " + aVals[i] + " " + ops[i] + " " + bVals[i] + " = " + result);
        }
    }
}