import java.nio.charset.StandardCharsets;

public class Task140 {
    public static final class CalcResult {
        public final boolean ok;
        public final double value;
        public final String error;

        public CalcResult(boolean ok, double value, String error) {
            this.ok = ok;
            this.value = value;
            this.error = error == null ? "" : error;
        }
    }

    public static CalcResult calculate(double a, double b, char op) {
        switch (op) {
            case '+':
                return new CalcResult(true, a + b, null);
            case '-':
                return new CalcResult(true, a - b, null);
            case '*':
                return new CalcResult(true, a * b, null);
            case '/':
                if (b == 0.0) {
                    return new CalcResult(false, 0.0, "Division by zero");
                }
                return new CalcResult(true, a / b, null);
            default:
                return new CalcResult(false, 0.0, "Invalid operator");
        }
    }

    public static void main(String[] args) {
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
    }

    private static void printResult(double a, double b, char op, CalcResult res) {
        if (res.ok) {
            System.out.printf("Input: %.6f %c %.6f => Result: %.6f%n", a, op, b, res.value);
        } else {
            System.out.printf("Input: %.6f %c %.6f => Error: %s%n", a, op, b, res.error);
        }
    }
}