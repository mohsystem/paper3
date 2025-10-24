public final class Task140 {
    public static final class CalcResult {
        private final boolean ok;
        private final double value;
        private final String error;

        private CalcResult(boolean ok, double value, String error) {
            this.ok = ok;
            this.value = value;
            this.error = error;
        }

        public static CalcResult success(double value) {
            return new CalcResult(true, value, null);
        }

        public static CalcResult failure(String error) {
            return new CalcResult(false, Double.NaN, error);
        }

        public boolean isOk() {
            return ok;
        }

        public double getValue() {
            return value;
        }

        public String getError() {
            return error;
        }
    }

    public static CalcResult calculate(double a, double b, char op) {
        switch (op) {
            case '+':
                return CalcResult.success(a + b);
            case '-':
                return CalcResult.success(a - b);
            case '*':
                return CalcResult.success(a * b);
            case '/':
                if (Double.compare(b, 0.0) == 0) {
                    return CalcResult.failure("Error: Division by zero.");
                }
                return CalcResult.success(a / b);
            default:
                return CalcResult.failure("Error: Unsupported operator. Use one of +, -, *, /.");
        }
    }

    public static void main(String[] args) {
        double[][] operands = {
            {10.0, 5.0},
            {10.0, 5.0},
            {10.0, 5.0},
            {10.0, 5.0},
            {10.0, 0.0}
        };
        char[] operators = {'+', '-', '*', '/', '/'};

        for (int i = 0; i < operators.length; i++) {
            double a = operands[i][0];
            double b = operands[i][1];
            char op = operators[i];
            CalcResult res = calculate(a, b, op);
            if (res.isOk()) {
                System.out.println(a + " " + op + " " + b + " = " + res.getValue());
            } else {
                System.out.println("Operation " + a + " " + op + " " + b + " failed: " + res.getError());
            }
        }
    }
}