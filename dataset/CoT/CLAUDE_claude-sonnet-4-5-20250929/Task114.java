
import java.util.Arrays;

public class Task114 {
    
    // Copy string safely
    public static String copyString(String source) {
        if (source == null) {
            return null;
        }
        return new String(source);
    }
    
    // Concatenate strings safely with validation
    public static String concatenateStrings(String str1, String str2) {
        if (str1 == null && str2 == null) {
            return "";
        }
        if (str1 == null) {
            return str2;
        }
        if (str2 == null) {
            return str1;
        }
        
        // Using StringBuilder for safe concatenation
        StringBuilder sb = new StringBuilder(str1.length() + str2.length());
        sb.append(str1);
        sb.append(str2);
        return sb.toString();
    }
    
    // Reverse string safely
    public static String reverseString(String input) {
        if (input == null || input.isEmpty()) {
            return input;
        }
        
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = input.length() - 1; i >= 0; i--) {
            sb.append(input.charAt(i));
        }
        return sb.toString();
    }
    
    // Convert to uppercase safely
    public static String toUpperCaseSafe(String input) {
        if (input == null) {
            return null;
        }
        return input.toUpperCase();
    }
    
    // Convert to lowercase safely
    public static String toLowerCaseSafe(String input) {
        if (input == null) {
            return null;
        }
        return input.toLowerCase();
    }
    
    // Get substring with bounds checking
    public static String getSubstringSafe(String input, int start, int end) {
        if (input == null) {
            return null;
        }
        if (start < 0 || end > input.length() || start > end) {
            throw new IllegalArgumentException("Invalid substring indices");
        }
        return input.substring(start, end);
    }
    
    // Replace substring safely
    public static String replaceSubstring(String input, String target, String replacement) {
        if (input == null || target == null || replacement == null) {
            return input;
        }
        if (target.isEmpty()) {
            return input;
        }
        return input.replace(target, replacement);
    }
    
    public static void main(String[] args) {
        System.out.println("=== String Operations Test Cases ===\\n");
        
        // Test Case 1: Copy String
        System.out.println("Test Case 1 - Copy String:");
        String original = "Hello, World!";
        String copied = copyString(original);
        System.out.println("Original: " + original);
        System.out.println("Copied: " + copied);
        System.out.println();
        
        // Test Case 2: Concatenate Strings
        System.out.println("Test Case 2 - Concatenate Strings:");
        String str1 = "Hello";
        String str2 = " Java";
        String concatenated = concatenateStrings(str1, str2);
        System.out.println("String 1: " + str1);
        System.out.println("String 2: " + str2);
        System.out.println("Concatenated: " + concatenated);
        System.out.println();
        
        // Test Case 3: Reverse String
        System.out.println("Test Case 3 - Reverse String:");
        String toReverse = "Programming";
        String reversed = reverseString(toReverse);
        System.out.println("Original: " + toReverse);
        System.out.println("Reversed: " + reversed);
        System.out.println();
        
        // Test Case 4: Case Conversion
        System.out.println("Test Case 4 - Case Conversion:");
        String mixedCase = "HeLLo WoRLd";
        String upper = toUpperCaseSafe(mixedCase);
        String lower = toLowerCaseSafe(mixedCase);
        System.out.println("Original: " + mixedCase);
        System.out.println("Uppercase: " + upper);
        System.out.println("Lowercase: " + lower);
        System.out.println();
        
        // Test Case 5: Substring and Replace
        System.out.println("Test Case 5 - Substring and Replace:");
        String text = "The quick brown fox jumps over the lazy dog";
        String substring = getSubstringSafe(text, 4, 9);
        String replaced = replaceSubstring(text, "fox", "cat");
        System.out.println("Original: " + text);
        System.out.println("Substring (4-9): " + substring);
        System.out.println("Replaced 'fox' with 'cat': " + replaced);
    }
}
