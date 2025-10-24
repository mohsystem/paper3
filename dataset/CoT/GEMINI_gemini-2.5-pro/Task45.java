class Task45 {

    /**
     * Performs division on two numbers provided as strings.
     * It handles potential errors like invalid number formats and division by zero.
     *
     * @param numeratorStr The string representation of the numerator.
     * @param denominatorStr The string representation of the denominator.
     * @return A string containing the result or an error message.
     */
    public static String performDivision(String numeratorStr, String denominatorStr) {
        try {
            // Attempt to parse the strings into double values.
            // This can throw a NumberFormatException if the string is not a valid number.
            double numerator = Double.parseDouble(numeratorStr);
            double denominator = Double.parseDouble(denominatorStr);

            // Check for division by zero, which is a logical error.
            // Double division by zero results in "Infinity", but we'll treat it as an error.
            if (denominator == 0) {
                // Throw a specific exception for this case.
                throw new IllegalArgumentException("Cannot divide by zero.");
            }

            double result = numerator / denominator;
            return "Result: " + result;

        } catch (NumberFormatException e) {
            // Catch the error if parsing fails.
            return "Error: Invalid number format. Please provide valid numbers.";
        } catch (IllegalArgumentException e) {
            // Catch the specific division by zero error.
            return "Error: " + e.getMessage();
        } catch (Exception e) {
            // A general catch block for any other unexpected errors.
            return "An unexpected error occurred: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Valid division
        System.out.println("Test 1 ('10', '2'): " + performDivision("10", "2"));

        // Test Case 2: Division by zero
        System.out.println("Test 2 ('5', '0'): " + performDivision("5", "0"));

        // Test Case 3: Invalid numerator
        System.out.println("Test 3 ('abc', '5'): " + performDivision("abc", "5"));

        // Test Case 4: Invalid denominator
        System.out.println("Test 4 ('10', 'xyz'): " + performDivision("10", "xyz"));

        // Test Case 5: Valid floating-point division
        System.out.println("Test 5 ('7.5', '1.5'): " + performDivision("7.5", "1.5"));
    }
}