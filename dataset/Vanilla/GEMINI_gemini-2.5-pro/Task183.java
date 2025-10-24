public class Task183 {

    /**
     * Computes the largest product of a series of adjacent digits of a specified span.
     *
     * @param digits The sequence of digits to analyze.
     * @param span   The number of digits in each series.
     * @return The largest product found.
     * @throws IllegalArgumentException if the span is invalid or the input string contains non-digit characters.
     */
    public static long largestProduct(String digits, int span) {
        if (span < 0 || span > digits.length()) {
            throw new IllegalArgumentException("Span must be non-negative and not greater than the length of the digit string.");
        }

        for (char c : digits.toCharArray()) {
            if (!Character.isDigit(c)) {
                throw new IllegalArgumentException("Digit string must contain only digits.");
            }
        }

        if (span == 0) {
            return 1; // The product of an empty set is conventionally 1.
        }

        long maxProduct = 0L;

        for (int i = 0; i <= digits.length() - span; i++) {
            long currentProduct = 1L;
            for (int j = 0; j < span; j++) {
                currentProduct *= Character.getNumericValue(digits.charAt(i + j));
            }
            if (currentProduct > maxProduct) {
                maxProduct = currentProduct;
            }
        }

        return maxProduct;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println("Test Case 1: ('63915', 3)");
        System.out.println("Result: " + largestProduct("63915", 3)); // Expected: 162
        System.out.println();

        // Test Case 2
        System.out.println("Test Case 2: ('123456789', 4)");
        System.out.println("Result: " + largestProduct("123456789", 4)); // Expected: 3024
        System.out.println();

        // Test Case 3
        System.out.println("Test Case 3: ('1110987', 3)");
        System.out.println("Result: " + largestProduct("1110987", 3)); // Expected: 504
        System.out.println();
        
        // Test Case 4
        System.out.println("Test Case 4: ('12345', 5)");
        System.out.println("Result: " + largestProduct("12345", 5)); // Expected: 120
        System.out.println();

        // Test Case 5 (Invalid)
        System.out.println("Test Case 5: ('123', 4)");
        try {
            largestProduct("123", 4);
        } catch (IllegalArgumentException e) {
            System.out.println("Result: " + e.getMessage()); // Expected: Exception
        }
        System.out.println();
    }
}