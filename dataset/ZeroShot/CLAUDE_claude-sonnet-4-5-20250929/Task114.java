
public class Task114 {
    // String copy operation
    public static String copyString(String source) {
        if (source == null) {
            return null;
        }
        return new String(source);
    }
    
    // String concatenation operation
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
        return str1 + str2;
    }
    
    // String reverse operation
    public static String reverseString(String input) {
        if (input == null || input.isEmpty()) {
            return input;
        }
        return new StringBuilder(input).reverse().toString();
    }
    
    // String uppercase operation
    public static String toUpperCaseString(String input) {
        if (input == null) {
            return null;
        }
        return input.toUpperCase();
    }
    
    // String lowercase operation
    public static String toLowerCaseString(String input) {
        if (input == null) {
            return null;
        }
        return input.toLowerCase();
    }
    
    // String substring operation
    public static String getSubstring(String input, int start, int end) {
        if (input == null || start < 0 || end > input.length() || start > end) {
            return "";
        }
        return input.substring(start, end);
    }
    
    // String replace operation
    public static String replaceString(String input, String target, String replacement) {
        if (input == null || target == null || replacement == null) {
            return input;
        }
        return input.replace(target, replacement);
    }
    
    // String length operation
    public static int getStringLength(String input) {
        if (input == null) {
            return 0;
        }
        return input.length();
    }
    
    public static void main(String[] args) {
        // Test Case 1: Copy string
        System.out.println("Test Case 1 - Copy String:");
        String original = "Hello World";
        String copied = copyString(original);
        System.out.println("Original: " + original);
        System.out.println("Copied: " + copied);
        System.out.println();
        
        // Test Case 2: Concatenate strings
        System.out.println("Test Case 2 - Concatenate Strings:");
        String str1 = "Hello";
        String str2 = " World";
        String concatenated = concatenateStrings(str1, str2);
        System.out.println("String 1: " + str1);
        System.out.println("String 2: " + str2);
        System.out.println("Concatenated: " + concatenated);
        System.out.println();
        
        // Test Case 3: Reverse string
        System.out.println("Test Case 3 - Reverse String:");
        String text = "Programming";
        String reversed = reverseString(text);
        System.out.println("Original: " + text);
        System.out.println("Reversed: " + reversed);
        System.out.println();
        
        // Test Case 4: Case conversion and substring
        System.out.println("Test Case 4 - Case Conversion and Substring:");
        String message = "Secure Coding Practice";
        System.out.println("Original: " + message);
        System.out.println("Uppercase: " + toUpperCaseString(message));
        System.out.println("Lowercase: " + toLowerCaseString(message));
        System.out.println("Substring (0-6): " + getSubstring(message, 0, 6));
        System.out.println();
        
        // Test Case 5: Replace and length
        System.out.println("Test Case 5 - Replace and Length:");
        String sentence = "Java is great. Java is powerful.";
        String replaced = replaceString(sentence, "Java", "Python");
        System.out.println("Original: " + sentence);
        System.out.println("Replaced: " + replaced);
        System.out.println("Original Length: " + getStringLength(sentence));
        System.out.println("Replaced Length: " + getStringLength(replaced));
    }
}
