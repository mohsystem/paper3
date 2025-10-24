
public class Task183 {
    public static long largestProduct(String input, int span) {
        // Handle edge cases
        if (input == null || span < 0 || span > input.length()) {
            throw new IllegalArgumentException("Invalid input or span");
        }
        
        if (span == 0) {
            return 1;
        }
        
        // Remove non-digit characters
        String digits = input.replaceAll("[^0-9]", "");
        
        if (span > digits.length()) {
            throw new IllegalArgumentException("Span is larger than input length");
        }
        
        long maxProduct = 0;
        
        // Iterate through all possible series
        for (int i = 0; i <= digits.length() - span; i++) {
            long product = 1;
            for (int j = i; j < i + span; j++) {
                product *= (digits.charAt(j) - '0');
            }
            maxProduct = Math.max(maxProduct, product);
        }
        
        return maxProduct;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + largestProduct("63915", 3)); // Expected: 162
        
        // Test case 2
        System.out.println("Test 2: " + largestProduct("123456789", 4)); // Expected: 3024
        
        // Test case 3
        System.out.println("Test 3: " + largestProduct("0123456789", 3)); // Expected: 504
        
        // Test case 4
        System.out.println("Test 4: " + largestProduct("99999", 2)); // Expected: 81
        
        // Test case 5
        System.out.println("Test 5: " + largestProduct("1234", 1)); // Expected: 4
    }
}
