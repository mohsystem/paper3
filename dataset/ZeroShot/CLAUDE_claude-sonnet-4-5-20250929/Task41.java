
import java.util.Scanner;

public class Task41 {
    private static final int MAX_INPUT_LENGTH = 1024;
    
    public static String processString(String input) {
        if (input == null) {
            return "Error: Null input";
        }
        
        // Validate input length
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + MAX_INPUT_LENGTH;
        }
        
        // Process the string (example: convert to uppercase and trim)
        String processed = input.trim().toUpperCase();
        return "Processed: " + processed;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1: " + processString("Hello World"));
        
        // Test case 2: Empty string
        System.out.println("Test 2: " + processString(""));
        
        // Test case 3: String with whitespace
        System.out.println("Test 3: " + processString("  Java Programming  "));
        
        // Test case 4: Null input
        System.out.println("Test 4: " + processString(null));
        
        // Test case 5: Very long string
        StringBuilder longString = new StringBuilder();
        for (int i = 0; i < 2000; i++) {
            longString.append("A");
        }
        System.out.println("Test 5: " + processString(longString.toString()));
    }
}
