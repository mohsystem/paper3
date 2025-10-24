
import java.nio.charset.StandardCharsets;

public class Task58 {
    public static boolean XO(String input) {
        // Validate input is not null
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        // Validate input length to prevent excessive processing
        if (input.length() > 100000) {
            throw new IllegalArgumentException("Input length exceeds maximum allowed");
        }
        
        int xCount = 0;
        int oCount = 0;
        
        // Convert to bytes using UTF-8 to ensure safe processing
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        
        for (byte b : bytes) {
            // Convert to lowercase for case-insensitive comparison
            char c = Character.toLowerCase((char) b);
            
            if (c == 'x') {
                xCount++;
            } else if (c == 'o') {
                oCount++;
            }
        }
        
        return xCount == oCount;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1 - XO(\\"ooxx\\"): " + XO("ooxx") + " (expected: true)");
        
        // Test case 2
        System.out.println("Test 2 - XO(\\"xooxx\\"): " + XO("xooxx") + " (expected: false)");
        
        // Test case 3
        System.out.println("Test 3 - XO(\\"ooxXm\\"): " + XO("ooxXm") + " (expected: true)");
        
        // Test case 4
        System.out.println("Test 4 - XO(\\"zpzpzpp\\"): " + XO("zpzpzpp") + " (expected: true)");
        
        // Test case 5
        System.out.println("Test 5 - XO(\\"zzoo\\"): " + XO("zzoo") + " (expected: false)");
    }
}
