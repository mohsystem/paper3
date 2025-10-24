
import java.util.Scanner;
import java.util.Arrays;

public class Task104 {
    private static final int BUFFER_SIZE = 100;
    
    /**
     * Safely handles user input into a fixed-size buffer
     * @param input The user input string
     * @return The safely buffered string
     */
    public static String handleInputToBuffer(String input) {
        if (input == null) {
            input = "";
        }
        
        // Create fixed-size buffer
        char[] buffer = new char[BUFFER_SIZE];
        Arrays.fill(buffer, '\\0');
        
        // Safely copy input to buffer with bounds checking
        int copyLength = Math.min(input.length(), BUFFER_SIZE - 1);
        for (int i = 0; i < copyLength; i++) {
            buffer[i] = input.charAt(i);
        }
        
        // Null-terminate (already done with Arrays.fill)
        buffer[copyLength] = '\\0';
        
        // Convert to string, stopping at null terminator
        return new String(buffer, 0, copyLength);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Buffer Input Handler - Java ===\\n");
        
        // Test case 1: Normal input
        String test1 = "Hello, World!";
        System.out.println("Test 1 - Normal input:");
        System.out.println("Input: " + test1);
        System.out.println("Output: " + handleInputToBuffer(test1));
        System.out.println();
        
        // Test case 2: Empty input
        String test2 = "";
        System.out.println("Test 2 - Empty input:");
        System.out.println("Input: (empty)");
        System.out.println("Output: " + handleInputToBuffer(test2));
        System.out.println();
        
        // Test case 3: Input at buffer limit
        String test3 = "A".repeat(99);
        System.out.println("Test 3 - Input at buffer limit (99 chars):");
        System.out.println("Input length: " + test3.length());
        System.out.println("Output length: " + handleInputToBuffer(test3).length());
        System.out.println();
        
        // Test case 4: Input exceeding buffer size (security test)
        String test4 = "B".repeat(150);
        System.out.println("Test 4 - Input exceeding buffer (150 chars):");
        System.out.println("Input length: " + test4.length());
        String result4 = handleInputToBuffer(test4);
        System.out.println("Output length: " + result4.length());
        System.out.println("Safely truncated: " + (result4.length() < test4.length()));
        System.out.println();
        
        // Test case 5: Null input
        String test5 = null;
        System.out.println("Test 5 - Null input:");
        System.out.println("Input: null");
        System.out.println("Output: " + handleInputToBuffer(test5));
        System.out.println();
    }
}
