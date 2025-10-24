
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task19 {
    public static String reverseWords(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        // Validate input contains only letters and spaces
        if (!input.matches("[a-zA-Z ]*")) {
            throw new IllegalArgumentException("Input must contain only letters and spaces");
        }
        
        // Validate reasonable length to prevent DoS
        if (input.length() > 10000) {
            throw new IllegalArgumentException("Input exceeds maximum allowed length");
        }
        
        String[] words = input.split(" ", -1);
        StringBuilder result = new StringBuilder();
        
        for (int i = 0; i < words.length; i++) {
            String word = words[i];
            if (word.length() >= 5) {
                result.append(reverseString(word));
            } else {
                result.append(word);
            }
            
            if (i < words.length - 1) {
                result.append(" ");
            }
        }
        
        return result.toString();
    }
    
    private static String reverseString(String str) {
        char[] chars = str.toCharArray();
        int left = 0;
        int right = chars.length - 1;
        
        while (left < right) {
            char temp = chars[left];
            chars[left] = chars[right];
            chars[right] = temp;
            left++;
            right--;
        }
        
        return new String(chars);
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "Hey fellow warriors";
        System.out.println("Input: \\"" + test1 + "\\"");
        System.out.println("Output: \\"" + reverseWords(test1) + "\\"");
        System.out.println();
        
        // Test case 2
        String test2 = "This is a test";
        System.out.println("Input: \\"" + test2 + "\\"");
        System.out.println("Output: \\"" + reverseWords(test2) + "\\"");
        System.out.println();
        
        // Test case 3
        String test3 = "This is another test";
        System.out.println("Input: \\"" + test3 + "\\"");
        System.out.println("Output: \\"" + reverseWords(test3) + "\\"");
        System.out.println();
        
        // Test case 4
        String test4 = "abcd";
        System.out.println("Input: \\"" + test4 + "\\"");
        System.out.println("Output: \\"" + reverseWords(test4) + "\\"");
        System.out.println();
        
        // Test case 5
        String test5 = "apple banana";
        System.out.println("Input: \\"" + test5 + "\\"");
        System.out.println("Output: \\"" + reverseWords(test5) + "\\"");
    }
}
