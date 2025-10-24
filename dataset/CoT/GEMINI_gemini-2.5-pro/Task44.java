import java.util.InputMismatchException;
import java.util.Scanner;

public class Task44 {

    /**
     * Performs a basic arithmetic operation after validating inputs.
     *
     * @param num1     The first integer.
     * @param num2     The second integer.
     * @param operator The character representing the operation (+, -, *, /).
     * @return A string representing the result or an error message.
     */
    public static String performOperation(int num1, int num2, char operator) {
        double result = 0.0;
        switch (operator) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                // Security: Validate against division by zero.
                if (num2 == 0) {
                    return "Error: Division by zero is not allowed.";
                }
                result = (double) num1 / num2;
                break;
            default:
                // Security: Validate the operator is one of the allowed characters.
                return "Error: Invalid operator '" + operator + "'.";
        }
        return String.format("%d %c %d = %.2f", num1, operator, num2, result);
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running Automated Test Cases ---");

        // Test Case 1: Addition
        System.out.println("Test Case 1 (Addition):");
        System.out.println("Input: 10, 5, '+'");
        System.out.println("Output: " + performOperation(10, 5, '+'));
        System.out.println();

        // Test Case 2: Subtraction
        System.out.println("Test Case 2 (Subtraction):");
        System.out.println("Input: 10, 15, '-'");
        System.out.println("Output: " + performOperation(10, 15, '-'));
        System.out.println();

        // Test Case 3: Valid Division
        System.out.println("Test Case 3 (Valid Division):");
        System.out.println("Input: 20, 4, '/'");
        System.out.println("Output: " + performOperation(20, 4, '/'));
        System.out.println();

        // Test Case 4: Invalid Division (by zero)
        System.out.println("Test Case 4 (Division by Zero):");
        System.out.println("Input: 10, 0, '/'");
        System.out.println("Output: " + performOperation(10, 0, '/'));
        System.out.println();

        // Test Case 5: Invalid Operator
        System.out.println("Test Case 5 (Invalid Operator):");
        System.out.println("Input: 7, 8, '%'");
        System.out.println("Output: " + performOperation(7, 8, '%'));
        System.out.println();
        
        // --- Interactive Section (Example of secure input handling) ---
        // Uncomment the block below to run interactively.
        /*
        System.out.println("\n--- Starting Interactive Session ---");
        Scanner scanner = new Scanner(System.in);
        int num1 = 0, num2 = 0;
        char op = ' ';
        boolean validInput = false;

        // Securely read first integer
        while (!validInput) {
            try {
                System.out.print("Enter first integer: ");
                num1 = scanner.nextInt();
                validInput = true;
            } catch (InputMismatchException e) {
                System.out.println("Error: Invalid input. Please enter an integer.");
                scanner.next(); // Clear the invalid input from the scanner buffer
            }
        }

        // Securely read second integer
        validInput = false;
        while (!validInput) {
            try {
                System.out.print("Enter second integer: ");
                num2 = scanner.nextInt();
                validInput = true;
            } catch (InputMismatchException e) {
                System.out.println("Error: Invalid input. Please enter an integer.");
                scanner.next();
            }
        }

        // Securely read operator
        validInput = false;
        while (!validInput) {
            System.out.print("Enter an operator (+, -, *, /): ");
            String input = scanner.next();
            if (input.length() == 1) {
                char firstChar = input.charAt(0);
                if (firstChar == '+' || firstChar == '-' || firstChar == '*' || firstChar == '/') {
                    op = firstChar;
                    validInput = true;
                } else {
                    System.out.println("Error: Invalid operator. Please use +, -, *, or /.");
                }
            } else {
                 System.out.println("Error: Please enter only a single operator character.");
            }
        }

        System.out.println("Result: " + performOperation(num1, num2, op));
        scanner.close();
        */
    }
}