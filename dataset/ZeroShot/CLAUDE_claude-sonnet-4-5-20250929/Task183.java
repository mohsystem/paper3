
public class Task183 {
    public static long largestProduct(String input, int span) {
        // Validate inputs
        if (input == null || input.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
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
        for (char c : input.toCharArray()) {
            if (!Character.isDigit(c)) {
                throw new IllegalArgumentException("Input must contain only digits");
            }
        }
        
        long maxProduct = 0;
        
        // Iterate through all possible series
        for (int i = 0; i <= input.length() - span; i++) {
            long product = 1;
            
            // Calculate product of current series
            for (int j = i; j < i + span; j++) {
                product *= (input.charAt(j) - '0');
            }
            
            // Update max product if current is larger
            maxProduct = Math.max(maxProduct, product);
        }
        
        return maxProduct;
    }
    
    public static void main(String[] args) {
        // Test case 1: Example from prompt
        System.out.println("Test 1: " + largestProduct("63915", 3)); // Expected: 162
        
        // Test case 2: Span of 1
        System.out.println("Test 2: " + largestProduct("63915", 1)); // Expected: 9
        
        // Test case 3: Span equals input length
        System.out.println("Test 3: " + largestProduct("123", 3)); // Expected: 6
        
        // Test case 4: With zeros
        System.out.println("Test 4: " + largestProduct("1203", 2)); // Expected: 6
        
        // Test case 5: Larger number
        System.out.println("Test 5: " + largestProduct("73167176531330624919225119674426574742355349194934", 6)); // Expected: 23520
    }
}
