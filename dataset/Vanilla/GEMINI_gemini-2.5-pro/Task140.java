public class Task140 {

    /**
     * Performs a basic arithmetic operation.
     *
     * @param num1     The first number.
     * @param num2     The second number.
     * @param operator The operator (+, -, *, /).
     * @return The result of the operation, or Double.NaN if the operation is invalid.
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
                if (num2 != 0) {
                    return num1 / num2;
                } else {
                    System.err.println("Error: Division by zero is not allowed.");
                    return Double.NaN; // Not a Number
                }
            default:
                System.err.println("Error: Invalid operator '" + operator + "'.");
                return Double.NaN;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Addition
        double num1_1 = 10, num2_1 = 5;
        char op1 = '+';
        System.out.println("Test Case 1: " + num1_1 + " " + op1 + " " + num2_1 + " = " + calculate(num1_1, num2_1, op1));

        // Test Case 2: Subtraction
        double num1_2 = 20.5, num2_2 = 10.5;
        char op2 = '-';
        System.out.println("Test Case 2: " + num1_2 + " " + op2 + " " + num2_2 + " = " + calculate(num1_2, num2_2, op2));

        // Test Case 3: Multiplication
        double num1_3 = 7, num2_3 = 8;
        char op3 = '*';
        System.out.println("Test Case 3: " + num1_3 + " " + op3 + " " + num2_3 + " = " + calculate(num1_3, num2_3, op3));

        // Test Case 4: Division
        double num1_4 = 100, num2_4 = 4;
        char op4 = '/';
        System.out.println("Test Case 4: " + num1_4 + " " + op4 + " " + num2_4 + " = " + calculate(num1_4, num2_4, op4));

        // Test Case 5: Division by zero
        double num1_5 = 15, num2_5 = 0;
        char op5 = '/';
        System.out.print("Test Case 5: " + num1_5 + " " + op5 + " " + num2_5 + " = ");
        double result5 = calculate(num1_5, num2_5, op5);
        // The error message is printed inside the function.
        // We can check for NaN to print a specific message for the result.
        if (Double.isNaN(result5)) {
            System.out.println("Undefined");
        } else {
            System.out.println(result5);
        }
    }
}