import java.util.regex.Pattern;

public class Task183 {

    /**
     * Computes the largest product of a series of adjacent digits.
     *
     * @param input The sequence of digits to analyze.
     * @param span The number of digits in each series.
     * @return The largest product.
     * @throws IllegalArgumentException if the input is invalid.
     */
    public static long largestProduct(String input, int span) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null.");
        }
        if (span < 0) {
            throw new IllegalArgumentException("Span cannot be negative.");
        }
        if (span > input.length()) {
            throw new IllegalArgumentException("Span cannot be larger than the input string length.");
        }
        if (!Pattern.matches("\\d*", input)) {
            throw new IllegalArgumentException("Input string must contain only digits.");
        }

        if (span == 0) {
            return 1;
        }

        long maxProduct = 0;

        for (int i = 0; i <= input.length() - span; i++) {
            long currentProduct = 1;
            for (int j = 0; j < span; j++) {
                // Character.getNumericValue is safer than 'char' - '0'
                currentProduct *= Character.getNumericValue(input.charAt(i + j));
            }
            if (currentProduct > maxProduct) {
                maxProduct = currentProduct;
            }
        }

        return maxProduct;
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        String input1 = "63915";
        int span1 = 3;
        System.out.println("Input: \"" + input1 + "\", Span: " + span1 + ", Largest Product: " + largestProduct(input1, span1)); // Expected: 162

        // Test Case 2: Contains a zero
        String input2 = "1027839564";
        int span2 = 5;
        System.out.println("Input: \"" + input2 + "\", Span: " + span2 + ", Largest Product: " + largestProduct(input2, span2)); // Expected: 22680 (3*9*5*6*4)

        // Test Case 3: Span equals string length
        String input3 = "73167176531330624919225119674426574742355349194934";
        int span3 = 6;
        System.out.println("Input: \"" + input3.substring(0,10) + "...\", Span: " + span3 + ", Largest Product: " + largestProduct(input3, span3)); // Expected: 23520

        // Test Case 4: Span is 0
        String input4 = "123";
        int span4 = 0;
        System.out.println("Input: \"" + input4 + "\", Span: " + span4 + ", Largest Product: " + largestProduct(input4, span4)); // Expected: 1

        // Test Case 5: Span is 1
        String input5 = "18395";
        int span5 = 1;
        System.out.println("Input: \"" + input5 + "\", Span: " + span5 + ", Largest Product: " + largestProduct(input5, span5)); // Expected: 9
        
        // Test Case 6: Invalid input (demonstrating exception handling)
        try {
            largestProduct("123a45", 3);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
    }
}