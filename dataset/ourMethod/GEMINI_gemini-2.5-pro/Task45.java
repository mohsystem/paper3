import java.util.InputMismatchException;
import java.util.Scanner;

public class Task45 {

    /**
     * Divides two integers and returns the result.
     *
     * @param numerator   The number to be divided.
     * @param denominator The number to divide by.
     * @return The result of the division as a double.
     * @throws IllegalArgumentException if the denominator is zero.
     */
    public static double divide(int numerator, int denominator) {
        if (denominator == 0) {
            throw new IllegalArgumentException("Error: Division by zero is not allowed.");
        }
        return (double) numerator / denominator;
    }

    /**
     * Processes two string inputs, converts them to integers, performs division,
     * and prints the result or an error message.
     *
     * @param numStr1 String representation of the numerator.
     * @param numStr2 String representation of the denominator.
     */
    public static void processDivision(String numStr1, String numStr2) {
        System.out.println("Processing inputs: \"" + numStr1 + "\", \"" + numStr2 + "\"");
        try {
            // Rule #1: Validate input by parsing.
            // Rule #7: Use safe parsing.
            int numerator = Integer.parseInt(numStr1);
            int denominator = Integer.parseInt(numStr2);

            // Perform the operation
            double result = divide(numerator, denominator);

            System.out.println("Result: " + result);

        // Rule #6: Ensure that all exceptions are caught and handled appropriately.
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid number format. Please enter integers only.");
        } catch (IllegalArgumentException e) {
            System.err.println(e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        } finally {
            System.out.println("--- Operation finished ---");
        }
    }


    public static void main(String[] args) {
        // Test cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        processDivision("10", "2");

        // Test Case 2: Division by zero
        processDivision("5", "0");

        // Test Case 3: Non-numeric input for numerator
        processDivision("abc", "5");

        // Test Case 4: Non-numeric input for denominator
        processDivision("10", "xyz");
        
        // Test Case 5: Large numbers (within integer limits)
        processDivision("2147483647", "2");
    }
}