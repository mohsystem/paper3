
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task114 {
    
    // Copy a string safely
    public static String copyString(String source) {
        if (source == null) {
            throw new IllegalArgumentException("Source string cannot be null");
        }
        // String is immutable in Java, so we create a new instance
        return new String(source.getBytes(StandardCharsets.UTF_8), StandardCharsets.UTF_8);
    }
    
    // Concatenate strings with validation
    public static String concatenateStrings(String str1, String str2) {
        if (str1 == null || str2 == null) {
            throw new IllegalArgumentException("Input strings cannot be null");
        }
        // Check combined length to prevent excessive memory allocation
        if (str1.length() > 10000 || str2.length() > 10000) {
            throw new IllegalArgumentException("String length exceeds maximum allowed size");
        }
        return str1 + str2;
    }
    
    // Reverse a string safely
    public static String reverseString(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null");
        }
        if (input.length() > 10000) {
            throw new IllegalArgumentException("String length exceeds maximum allowed size");
        }
        StringBuilder sb = new StringBuilder(input);
        return sb.reverse().toString();
    }
    
    // Convert to uppercase with validation
    public static String toUpperCaseSafe(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null");
        }
        if (input.length() > 10000) {
            throw new IllegalArgumentException("String length exceeds maximum allowed size");
        }
        return input.toUpperCase();
    }
    
    // Substring with bounds checking
    public static String substringSafe(String input, int start, int end) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null");
        }
        if (start < 0 || end < 0 || start > end || end > input.length()) {
            throw new IllegalArgumentException("Invalid substring indices");
        }
        return input.substring(start, end);
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Copy string
            System.out.println("Test 1 - Copy String:");
            String original1 = "Hello, World!";
            String copied = copyString(original1);
            System.out.println("Original: " + original1);
            System.out.println("Copied: " + copied);
            System.out.println();
            
            // Test case 2: Concatenate strings
            System.out.println("Test 2 - Concatenate Strings:");
            String str1 = "Hello, ";
            String str2 = "Java!";
            String concatenated = concatenateStrings(str1, str2);
            System.out.println("String 1: " + str1);
            System.out.println("String 2: " + str2);
            System.out.println("Concatenated: " + concatenated);
            System.out.println();
            
            // Test case 3: Reverse string
            System.out.println("Test 3 - Reverse String:");
            String toReverse = "Programming";
            String reversed = reverseString(toReverse);
            System.out.println("Original: " + toReverse);
            System.out.println("Reversed: " + reversed);
            System.out.println();
            
            // Test case 4: Convert to uppercase
            System.out.println("Test 4 - To Uppercase:");
            String toLower = "secure coding";
            String upper = toUpperCaseSafe(toLower);
            System.out.println("Original: " + toLower);
            System.out.println("Uppercase: " + upper);
            System.out.println();
            
            // Test case 5: Substring
            System.out.println("Test 5 - Substring:");
            String fullString = "Security First";
            String sub = substringSafe(fullString, 0, 8);
            System.out.println("Original: " + fullString);
            System.out.println("Substring [0-8]: " + sub);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
