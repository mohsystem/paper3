import java.util.InputMismatchException;

public class Task45 {

    /**
     * Securely converts two string inputs to doubles and performs division.
     * It handles null/empty inputs, non-numeric formats, and division by zero.
     *
     * @param numStr1 The string representing the numerator.
     * @param numStr2 The string representing the denominator.
     * @return The result of the division, or Double.NaN if an error occurs.
     */
    public static double performDivision(String numStr1, String numStr2) {
        try {
            // 1. Validate for null or empty inputs
            if (numStr1 == null || numStr1.trim().isEmpty() || numStr2 == null || numStr2.trim().isEmpty()) {
                throw new IllegalArgumentException("Input strings cannot be null or empty.");
            }

            // 2. Convert strings to numbers, handling format errors
            double numerator = Double.parseDouble(numStr1);
            double denominator = Double.parseDouble(numStr2);

            // 3. Check for division by zero
            if (denominator == 0) {
                throw new ArithmeticException("Cannot divide by zero.");
            }

            return numerator / denominator;

        } catch (NumberFormatException e) {
            // Catches errors from Double.parseDouble for invalid number formats
            System.err.println("Error: Invalid number format provided. Please provide valid numbers.");
            return Double.NaN;
        } catch (IllegalArgumentException e) {
            // Catches our custom validation error
            System.err.println("Error: " + e.getMessage());
            return Double.NaN;
        } catch (ArithmeticException e) {
            // Catches division by zero
            System.err.println("Error: " + e.getMessage());
            return Double.NaN;
        } catch (Exception e) {
            // A general catch-all for any other unexpected errors
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return Double.NaN;
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Valid input
        System.out.println("Test Case 1: Valid input ('100', '20')");
        double result1 = performDivision("100", "20");
        if (!Double.isNaN(result1)) {
            System.out.println("Result: " + result1);
        }
        System.out.println("--------------------");

        // Test Case 2: Division by zero
        System.out.println("Test Case 2: Division by zero ('10', '0')");
        double result2 = performDivision("10", "0");
        if (!Double.isNaN(result2)) {
            System.out.println("Result: " + result2);
        }
        System.out.println("--------------------");

        // Test Case 3: Invalid number format
        System.out.println("Test Case 3: Invalid number format ('abc', '5')");
        double result3 = performDivision("abc", "5");
        if (!Double.isNaN(result3)) {
            System.out.println("Result: " + result3);
        }
        System.out.println("--------------------");

        // Test Case 4: Null input
        System.out.println("Test Case 4: Null input ('50', null)");
        double result4 = performDivision("50", null);
        if (!Double.isNaN(result4)) {
            System.out.println("Result: " + result4);
        }
        System.out.println("--------------------");

        // Test Case 5: Empty string input
        System.out.println("Test Case 5: Empty string input ('', '10')");
        double result5 = performDivision("", "10");
        if (!Double.isNaN(result5)) {
            System.out.println("Result: " + result5);
        }
        System.out.println("--------------------");
    }
}