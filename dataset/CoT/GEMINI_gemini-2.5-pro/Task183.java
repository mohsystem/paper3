public class Task183 {

    /**
     * Computes the largest product of a series of adjacent digits of a specified span.
     *
     * @param input The sequence of digits to analyze.
     * @param span  The number of digits in each series.
     * @return The largest product.
     * @throws IllegalArgumentException if the input is invalid.
     */
    public static long largestProduct(String input, int span) {
        // 1. Input Validation
        if (span < 0) {
            throw new IllegalArgumentException("Span must not be negative.");
        }
        if (input == null || span > input.length()) {
            throw new IllegalArgumentException("Span must be smaller than or equal to the length of the input string.");
        }
        for (char c : input.toCharArray()) {
            if (!Character.isDigit(c)) {
                throw new IllegalArgumentException("Input string must only contain digits.");
            }
        }

        // 2. Handle edge case: span is 0, the product of an empty set is 1.
        if (span == 0) {
            return 1;
        }

        // 3. Main Algorithm
        long maxProduct = 0;

        for (int i = 0; i <= input.length() - span; i++) {
            long currentProduct = 1;
            for (int j = 0; j < span; j++) {
                // Character.getNumericValue() converts char '0'-'9' to int 0-9
                int digit = Character.getNumericValue(input.charAt(i + j));
                currentProduct *= digit;
            }
            if (currentProduct > maxProduct) {
                maxProduct = currentProduct;
            }
        }
        return maxProduct;
    }

    public static void main(String[] args) {
        // Test Cases
        String[] inputs = {
                "63915",
                "1027839564",
                "12345",
                "12345",
                "73167176531330624919225119674426574742355349194934"
        };
        int[] spans = {3, 4, 5, 0, 6};
        long[] expectedOutputs = {162, 5832, 120, 1, 23520};

        for (int i = 0; i < inputs.length; i++) {
            try {
                long result = largestProduct(inputs[i], spans[i]);
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("Input: \"" + inputs[i] + "\", Span: " + spans[i]);
                System.out.println("Result: " + result);
                System.out.println("Expected: " + expectedOutputs[i]);
                System.out.println("Status: " + (result == expectedOutputs[i] ? "Passed" : "Failed"));
            } catch (IllegalArgumentException e) {
                System.out.println("Test Case " + (i + 1) + " threw an exception: " + e.getMessage());
            }
            System.out.println("--------------------");
        }

        // Test invalid cases
        try {
             largestProduct("123", 4);
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 6 (Invalid Span): Passed - " + e.getMessage());
        }
        try {
             largestProduct("1a2", 2);
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 7 (Invalid Chars): Passed - " + e.getMessage());
        }
    }
}