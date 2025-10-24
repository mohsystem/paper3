public class Task183 {

    /**
     * Computes the largest product of a series of adjacent digits.
     *
     * @param input The sequence of digits as a string.
     * @param span  The number of digits in each series.
     * @return The largest product found.
     * @throws IllegalArgumentException if the input is invalid (e.g., contains non-digits,
     *                                  span is negative, or span is larger than the input length).
     */
    public static long largestProduct(String input, int span) {
        // 1. Input Validation
        if (span < 0) {
            throw new IllegalArgumentException("Span cannot be negative.");
        }
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null.");
        }
        if (!input.matches("\\d*")) {
            throw new IllegalArgumentException("Input string must contain only digits.");
        }
        if (span > input.length()) {
            throw new IllegalArgumentException("Span cannot be greater than the input string length.");
        }

        // 2. Edge Case
        if (span == 0) {
            return 1;
        }

        // 3. Calculation
        long maxProduct = 0;

        for (int i = 0; i <= input.length() - span; i++) {
            long currentProduct = 1;
            for (int j = 0; j < span; j++) {
                // Convert character to its numeric value and multiply
                currentProduct *= Character.getNumericValue(input.charAt(i + j));
            }

            if (currentProduct > maxProduct) {
                maxProduct = currentProduct;
            }
        }
        return maxProduct;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        try {
            // Test Case 1: Example from prompt
            System.out.println(largestProduct("63915", 3));

            // Test Case 2: A sequence containing zero
            System.out.println(largestProduct("1234560789", 5));

            // Test Case 3: Span of 1
            System.out.println(largestProduct("987654321", 1));

            // Test Case 4: Span equals the length of the input
            System.out.println(largestProduct("12345", 5));

            // Test Case 5: Span of 0
            System.out.println(largestProduct("111111", 0));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}