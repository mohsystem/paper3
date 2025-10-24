public class Task140 {

    /**
     * Performs a basic arithmetic operation on two numbers.
     *
     * @param num1     The first number.
     * @param num2     The second number.
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
                    throw new IllegalArgumentException("Division by zero is not allowed.");
                }
                return num1 / num2;
            default:
                throw new IllegalArgumentException("Invalid operator: " + operator);
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Addition
        try {
            System.out.println("Test 1: 10.5 + 2.5 = " + calculate(10.5, 2.5, '+'));
        } catch (IllegalArgumentException e) {
            System.out.println("Test 1 Error: " + e.getMessage());
        }

        // Test Case 2: Subtraction
        try {
            System.out.println("Test 2: 20.0 - 5.5 = " + calculate(20.0, 5.5, '-'));
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 Error: " + e.getMessage());
        }

        // Test Case 3: Multiplication
        try {
            System.out.println("Test 3: 5.0 * 4.0 = " + calculate(5.0, 4.0, '*'));
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 Error: " + e.getMessage());
        }

        // Test Case 4: Division by Zero (Error)
        try {
            System.out.println("Test 4: 10.0 / 0.0 = " + calculate(10.0, 0.0, '/'));
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 Error: " + e.getMessage());
        }

        // Test Case 5: Invalid Operator (Error)
        try {
            System.out.println("Test 5: 10.0 % 5.0 = " + calculate(10.0, 5.0, '%'));
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 Error: " + e.getMessage());
        }
    }
}