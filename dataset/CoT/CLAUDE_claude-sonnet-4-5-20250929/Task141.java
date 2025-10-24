
import java.util.Scanner;

public class Task141 {
    /**
     * Reverses a given string safely
     * @param input the string to reverse
     * @return the reversed string, or empty string if input is null
     */
    public static String reverseString(String input) {
        // Security: Handle null input to prevent NullPointerException
        if (input == null) {
            return "";
        }
        
        // Security: Use StringBuilder for efficient and safe string manipulation
        StringBuilder reversed = new StringBuilder(input.length());
        
        // Reverse the string character by character
        for (int i = input.length() - 1; i >= 0; i--) {
            reversed.append(input.charAt(i));
        }
        
        return reversed.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        String test1 = "hello";
        System.out.println("Input: " + test1);
        System.out.println("Output: " + reverseString(test1));
        System.out.println();
        
        System.out.println("Test Case 2:");
        String test2 = "Java Programming";
        System.out.println("Input: " + test2);
        System.out.println("Output: " + reverseString(test2));
        System.out.println();
        
        System.out.println("Test Case 3:");
        String test3 = "12345";
        System.out.println("Input: " + test3);
        System.out.println("Output: " + reverseString(test3));
        System.out.println();
        
        System.out.println("Test Case 4:");
        String test4 = "";
        System.out.println("Input: (empty string)");
        System.out.println("Output: " + reverseString(test4));
        System.out.println();
        
        System.out.println("Test Case 5:");
        String test5 = null;
        System.out.println("Input: null");
        System.out.println("Output: " + reverseString(test5));
    }
}
