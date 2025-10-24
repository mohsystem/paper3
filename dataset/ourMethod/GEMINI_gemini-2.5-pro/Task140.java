import java.util.Locale;

public class Task140 {

    /**
     * Performs a basic arithmetic operation.
     *
     * @param num1     The first operand.
     * @param num2     The second operand.
     * @param operator The operator character (+, -, *, /).
     * @return The result of the operation.
     * @throws IllegalArgumentException if the operator is invalid or if division by zero is attempted.
     */
    public static double calculate(double num1, double num2, char operator) {
        switch (operator) {
            case '+':
                return num1 + num2;
            case '-':
                return num1 - num2;
            case '*':
                return num1 * num2;
            case '/':
                if (num2 == 0) {
                    throw new IllegalArgumentException("Error: Division by zero is not allowed.");
                }
                return num1 / num2;
            default:
                throw new IllegalArgumentException("Error: Invalid operator. Only +, -, *, / are allowed.");
        }
    }

    public static void main(String[] args) {
        // Use Locale.US to ensure dot is used as a decimal separator in printf
        Locale.setDefault(Locale.US);
        
        // Test cases
        double[][] numbers = {
            {10.0, 5.0},
            {15.0, 3.0},
            {7.5, 2.5},
            {12.0, 0.0}, // Division by zero case
            {10.0, 3.0}  // Invalid operator case
        };
        char[] operators = {'+', '*', '-', '/', '%'};

        for (int i = 0; i < 5; i++) {
            double num1 = numbers[i][0];
            double num2 = numbers[i][1];
            char op = operators[i];
            
            System.out.printf("Test Case %d: %.2f %c %.2f\n", i + 1, num1, op, num2);
            try {
                double result = calculate(num1, num2, op);
                System.out.printf("Result: %.2f\n\n", result);
            } catch (IllegalArgumentException e) {
                System.err.println("Error: " + e.getMessage() + "\n");
            }
        }
    }
}