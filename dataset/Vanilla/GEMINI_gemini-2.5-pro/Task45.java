public class Task45 {

    /**
     * Attempts to parse two strings as doubles and divide the first by the second.
     * Handles potential NumberFormatException and ArithmeticException.
     *
     * @param a The numerator as a string.
     * @param b The denominator as a string.
     * @return The result of the division as a Double, or null if an error occurred.
     */
    public static Double divideStrings(String a, String b) {
        try {
            double numerator = Double.parseDouble(a);
            double denominator = Double.parseDouble(b);

            if (denominator == 0) {
                // Manually throw an exception for division by zero
                throw new ArithmeticException("Division by zero is not allowed.");
            }

            return numerator / denominator;
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid number format. '" + e.getMessage() + "'");
            return null;
        } catch (ArithmeticException e) {
            System.err.println("Error: " + e.getMessage());
            return null;
        } catch (Exception e) {
            // A general catch-all for any other unexpected errors
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Valid division
        System.out.println("\nTest Case 1: divideStrings(\"10\", \"2\")");
        Double result1 = divideStrings("10", "2");
        if (result1 != null) {
            System.out.println("Result: " + result1);
        }

        // Test Case 2: Division by zero
        System.out.println("\nTest Case 2: divideStrings(\"5\", \"0\")");
        Double result2 = divideStrings("5", "0");
        if (result2 != null) {
            System.out.println("Result: " + result2);
        }

        // Test Case 3: Invalid numerator
        System.out.println("\nTest Case 3: divideStrings(\"abc\", \"5\")");
        Double result3 = divideStrings("abc", "5");
        if (result3 != null) {
            System.out.println("Result: " + result3);
        }

        // Test Case 4: Invalid denominator
        System.out.println("\nTest Case 4: divideStrings(\"10\", \"xyz\")");
        Double result4 = divideStrings("10", "xyz");
        if (result4 != null) {
            System.out.println("Result: " + result4);
        }

        // Test Case 5: Valid floating-point division
        System.out.println("\nTest Case 5: divideStrings(\"7.5\", \"2.5\")");
        Double result5 = divideStrings("7.5", "2.5");
        if (result5 != null) {
            System.out.println("Result: " + result5);
        }
    }
}