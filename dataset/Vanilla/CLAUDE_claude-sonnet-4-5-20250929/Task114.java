
public class Task114 {
    // Copy string
    public static String copyString(String source) {
        if (source == null) return null;
        return new String(source);
    }
    
    // Concatenate two strings
    public static String concatenateStrings(String str1, String str2) {
        if (str1 == null) str1 = "";
        if (str2 == null) str2 = "";
        return str1 + str2;
    }
    
    // Reverse a string
    public static String reverseString(String str) {
        if (str == null) return null;
        StringBuilder sb = new StringBuilder(str);
        return sb.reverse().toString();
    }
    
    // Convert to uppercase
    public static String toUpperCase(String str) {
        if (str == null) return null;
        return str.toUpperCase();
    }
    
    // Convert to lowercase
    public static String toLowerCase(String str) {
        if (str == null) return null;
        return str.toLowerCase();
    }
    
    // Get substring
    public static String getSubstring(String str, int start, int end) {
        if (str == null || start < 0 || end > str.length() || start > end) {
            return "";
        }
        return str.substring(start, end);
    }
    
    // Count character occurrences
    public static int countCharacter(String str, char ch) {
        if (str == null) return 0;
        int count = 0;
        for (int i = 0; i < str.length(); i++) {
            if (str.charAt(i) == ch) {
                count++;
            }
        }
        return count;
    }
    
    // Replace character
    public static String replaceCharacter(String str, char oldChar, char newChar) {
        if (str == null) return null;
        return str.replace(oldChar, newChar);
    }
    
    // Find string length
    public static int getLength(String str) {
        if (str == null) return 0;
        return str.length();
    }
    
    // Check if string is palindrome
    public static boolean isPalindrome(String str) {
        if (str == null) return false;
        String reversed = reverseString(str);
        return str.equals(reversed);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Copy and Concatenate ===");
        String original = "Hello";
        String copied = copyString(original);
        System.out.println("Original: " + original);
        System.out.println("Copied: " + copied);
        String concatenated = concatenateStrings("Hello", " World");
        System.out.println("Concatenated: " + concatenated);
        
        System.out.println("\\n=== Test Case 2: Reverse String ===");
        String str2 = "Programming";
        System.out.println("Original: " + str2);
        System.out.println("Reversed: " + reverseString(str2));
        
        System.out.println("\\n=== Test Case 3: Case Conversion ===");
        String str3 = "Java Python CPP";
        System.out.println("Original: " + str3);
        System.out.println("Uppercase: " + toUpperCase(str3));
        System.out.println("Lowercase: " + toLowerCase(str3));
        
        System.out.println("\\n=== Test Case 4: Substring and Character Operations ===");
        String str4 = "Hello World";
        System.out.println("Original: " + str4);
        System.out.println("Substring (0, 5): " + getSubstring(str4, 0, 5));
        System.out.println("Count 'l': " + countCharacter(str4, 'l'));
        System.out.println("Replace 'o' with '0': " + replaceCharacter(str4, 'o', '0'));
        System.out.println("Length: " + getLength(str4));
        
        System.out.println("\\n=== Test Case 5: Palindrome Check ===");
        String str5a = "radar";
        String str5b = "hello";
        System.out.println(str5a + " is palindrome: " + isPalindrome(str5a));
        System.out.println(str5b + " is palindrome: " + isPalindrome(str5b));
    }
}
