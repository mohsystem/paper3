
import java.util.Scanner;
import java.util.Arrays;

public class Task104 {
    private static final int BUFFER_SIZE = 100;
    
    /**
     * Safely handles user input into a fixed-size buffer
     * @param input The input string to be stored
     * @return The safely stored string (truncated if necessary)
     */
    public static String handleInput(String input) {
        if (input == null) {
            return "";
        }
        
        // Ensure input doesn't exceed buffer size\n        if (input.length() > BUFFER_SIZE) {\n            return input.substring(0, BUFFER_SIZE);\n        }\n        \n        return input;\n    }\n    \n    /**\n     * Stores input into a character buffer safely\n     * @param input The input string\n     * @return Character array buffer with safe content\n     */\n    public static char[] storeInBuffer(String input) {\n        char[] buffer = new char[BUFFER_SIZE];\n        Arrays.fill(buffer, '\\0');
        
        if (input == null || input.isEmpty()) {
            return buffer;
        }
        
        int length = Math.min(input.length(), BUFFER_SIZE);
        input.getChars(0, length, buffer, 0);
        
        return buffer;
    }
    
    public static void main(String[] args) {
        System.out.println("Testing Fixed-Size Buffer Input Handler\\n");
        
        // Test case 1: Normal input
        String test1 = "Hello, World!";
        System.out.println("Test 1 - Normal input:");
        System.out.println("Input: " + test1);
        System.out.println("Output: " + handleInput(test1));
        System.out.println("Buffer: " + new String(storeInBuffer(test1)).trim());
        System.out.println();
        
        // Test case 2: Empty input
        String test2 = "";
        System.out.println("Test 2 - Empty input:");
        System.out.println("Input: \\"\\"");
        System.out.println("Output: " + handleInput(test2));
        System.out.println();
        
        // Test case 3: Input exceeding buffer size
        String test3 = "A".repeat(150);
        System.out.println("Test 3 - Input exceeding buffer size (150 chars):");
        System.out.println("Input length: " + test3.length());
        String result3 = handleInput(test3);
        System.out.println("Output length: " + result3.length());
        System.out.println("Truncated: " + (test3.length() > result3.length()));
        System.out.println();
        
        // Test case 4: Input at buffer boundary
        String test4 = "B".repeat(100);
        System.out.println("Test 4 - Input at buffer boundary (100 chars):");
        System.out.println("Input length: " + test4.length());
        String result4 = handleInput(test4);
        System.out.println("Output length: " + result4.length());
        System.out.println();
        
        // Test case 5: Null input
        String test5 = null;
        System.out.println("Test 5 - Null input:");
        System.out.println("Input: null");
        System.out.println("Output: \\"" + handleInput(test5) + "\\"");
    }
}
