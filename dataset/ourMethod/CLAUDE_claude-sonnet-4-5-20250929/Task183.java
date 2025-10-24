
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;

public class Task183 {
    public static long largestProduct(String input, int span) {
        // Validate inputs
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        if (span < 0) {
            throw new IllegalArgumentException("Span cannot be negative");
        }
        if (span == 0) {
            return 1;
        }
        if (span > input.length()) {
            throw new IllegalArgumentException("Span cannot be larger than input length");
        }
        
        // Validate that input contains only digits
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c < '0' || c > '9') {
                throw new IllegalArgumentException("Input must contain only digits");
            }
        }
        
        long maxProduct = 0;
        
        // Calculate product for each series
        for (int i = 0; i <= input.length() - span; i++) {
            long product = 1;
            for (int j = i; j < i + span; j++) {
                int digit = input.charAt(j) - '0';
                product *= digit;
            }
            if (product > maxProduct) {
                maxProduct = product;
            }
        }
        
        return maxProduct;
    }
    
    public static void main(String[] args) {
        // Test cases
        List<TestCase> testCases = Arrays.asList(
            new TestCase("63915", 3, 162),
            new TestCase("0123456789", 5, 15120),
            new TestCase("99999", 2, 81),
            new TestCase("12345", 1, 5),
            new TestCase("1", 1, 1)
        );
        
        for (int i = 0; i < testCases.size(); i++) {
            TestCase tc = testCases.get(i);
            try {
                long result = largestProduct(tc.input, tc.span);
                String status = (result == tc.expected) ? "PASS" : "FAIL";
                System.out.println("Test " + (i + 1) + ": " + status + 
                                   " (Expected: " + tc.expected + ", Got: " + result + ")");
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + ": ERROR - " + e.getMessage());
            }
        }
    }
    
    static class TestCase {
        String input;
        int span;
        long expected;
        
        TestCase(String input, int span, long expected) {
            this.input = input;
            this.span = span;
            this.expected = expected;
        }
    }
}
