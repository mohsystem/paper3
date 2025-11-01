
import java.nio.charset.StandardCharsets;

public class Lse123 {
    public static String processString(String input) {
        // Validate input is not null
        if (input == null) {
            return null;
        }
        
        // Validate input length to prevent excessive memory usage
        final int MAX_LENGTH = 1048576; // 1MB limit
        if (input.length() > MAX_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum allowed length");
        }
        
        // Convert to char array for processing
        char[] chars = input.toCharArray();
        int length = chars.length;
        
        // First loop: trim trailing whitespace
        // Iterate from end to find last non-whitespace character
        while (length > 0 && Character.isWhitespace(chars[length - 1])) {
            length--;
        }
        
        // Second loop: trim trailing newlines (already handled by whitespace check above)
        // This is redundant but follows the specification
        while (length > 0 && (chars[length - 1] == '\\n' || chars[length - 1] == '\\r')) {
            length--;
        }
        
        // Create new string with trimmed content (automatically null-terminated in Java)
        return new String(chars, 0, length);
    }
    
    public static void main(String[] args) {
        // Check that exactly one argument is provided
        if (args.length != 1) {
            System.err.println("Error: Exactly one argument required");
            System.exit(1);
        }
        
        // Check the argument is a string (in Java, args are always strings)
        String input = args[0];
        if (input == null) {
            System.err.println("Error: Argument must be a valid string");
            System.exit(1);
        }
        
        // Process the string
        String result = processString(input);
        System.out.println("Result: '" + result + "'");
        
        // Test cases
        System.out.println("\\n=== Test Cases ===");
        
        // Test 1: String with trailing spaces
        String test1 = "hello world   ";
        System.out.println("Test 1 Input: '" + test1 + "'");
        System.out.println("Test 1 Output: '" + processString(test1) + "'");
        
        // Test 2: String with trailing newlines
        String test2 = "hello world\\n\\n";
        System.out.println("Test 2 Input: '" + test2.replace("\\n", "\\\\n") + "'");
        System.out.println("Test 2 Output: '" + processString(test2) + "'");
        
        // Test 3: String with mixed trailing whitespace
        String test3 = "hello world \\n \\t ";
        System.out.println("Test 3 Input: '" + test3.replace("\\n", "\\\\n").replace("\\t", "\\\\t") + "'");
        System.out.println("Test 3 Output: '" + processString(test3) + "'");
        
        // Test 4: String with no trailing whitespace
        String test4 = "hello world";
        System.out.println("Test 4 Input: '" + test4 + "'");
        System.out.println("Test 4 Output: '" + processString(test4) + "'");
        
        // Test 5: Empty string
        String test5 = "";
        System.out.println("Test 5 Input: '" + test5 + "'");
        System.out.println("Test 5 Output: '" + processString(test5) + "'");
    }
}
