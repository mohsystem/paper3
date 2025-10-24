
public class Task5 {
    public static int persistence(long num) {
        // Input validation: ensure num is positive
        if (num < 0) {
            throw new IllegalArgumentException("Number must be positive");
        }
        
        // Base case: single digit
        if (num < 10) {
            return 0;
        }
        
        int count = 0;
        
        // Continue until we reach a single digit
        while (num >= 10) {
            long product = 1;
            
            // Multiply all digits
            while (num > 0) {
                product *= num % 10;
                num /= 10;
            }
            
            num = product;
            count++;
        }
        
        return count;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: persistence(39) = " + persistence(39) + " (expected: 3)");
        System.out.println("Test 2: persistence(999) = " + persistence(999) + " (expected: 4)");
        System.out.println("Test 3: persistence(4) = " + persistence(4) + " (expected: 0)");
        System.out.println("Test 4: persistence(25) = " + persistence(25) + " (expected: 2)");
        System.out.println("Test 5: persistence(10) = " + persistence(10) + " (expected: 1)");
    }
}
