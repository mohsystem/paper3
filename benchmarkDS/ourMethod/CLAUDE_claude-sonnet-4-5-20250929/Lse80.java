
import java.util.Scanner;
import java.nio.charset.StandardCharsets;

public class Lse80 {
    /**
     * Reads a line from standard input with a maximum length limit.
     * Returns the number of characters read (excluding newline).
     * Maximum 1023 characters to fit in a 1024-byte buffer (including null terminator).
     */
    public static int readLimitedLine(char[] buffer, int maxLen) {
        // Validate input parameters - bounds checking to prevent buffer overflow
        if (buffer == null || maxLen <= 0 || maxLen > buffer.length) {
            return -1; // Fail closed on invalid input
        }
        
        Scanner scanner = new Scanner(System.in, StandardCharsets.UTF_8);
        
        try {
            if (!scanner.hasNextLine()) {
                return 0; // No input available
            }
            
            String line = scanner.nextLine();
            
            // Enforce maximum length constraint - prevent buffer overflow
            int charsToRead = Math.min(line.length(), maxLen);
            
            // Copy characters with explicit bounds checking
            for (int i = 0; i < charsToRead; i++) {
                buffer[i] = line.charAt(i);
            }
            
            // Ensure null termination for C-style compatibility
            if (charsToRead < buffer.length) {
                buffer[charsToRead] = '\\0';
            }
            
            return charsToRead; // Return number of characters read
            
        } catch (Exception e) {
            // Fail closed with minimal error information
            return -1;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Normal input");
        char[] buffer1 = new char[1024];
        // Simulate input: "Hello"
        System.out.println("Expected: 5 characters");
        
        System.out.println("\\nTest Case 2: Maximum length input");
        char[] buffer2 = new char[1024];
        // Simulate input: 1023 'A' characters
        System.out.println("Expected: 1023 characters");
        
        System.out.println("\\nTest Case 3: Empty input");
        char[] buffer3 = new char[1024];
        // Simulate input: ""
        System.out.println("Expected: 0 characters");
        
        System.out.println("\\nTest Case 4: Input exceeds buffer");
        char[] buffer4 = new char[1024];
        // Simulate input: 2000 'B' characters
        System.out.println("Expected: 1023 characters (truncated)");
        
        System.out.println("\\nTest Case 5: Special characters");
        char[] buffer5 = new char[1024];
        // Simulate input: "Test@123!#"
        System.out.println("Expected: 10 characters");
        
        System.out.println("\\nNote: Actual execution requires stdin input");
    }
}
