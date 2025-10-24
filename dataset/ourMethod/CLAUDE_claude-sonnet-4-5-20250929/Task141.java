
import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class Task141 {
    private static final int MAX_INPUT_LENGTH = 10000;
    
    public static String reverseString(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length of " + MAX_INPUT_LENGTH);
        }
        
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        StringBuilder reversed = new StringBuilder(input.length());
        
        for (int i = input.length() - 1; i >= 0; i--) {
            reversed.append(input.charAt(i));
        }
        
        return reversed.toString();
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "hello",
            "world",
            "12345",
            "a",
            "racecar"
        };
        
        for (String testCase : testCases) {
            try {
                String result = reverseString(testCase);
                System.out.println("Input: \\"" + testCase + "\\" -> Output: \\"" + result + "\\"");
            } catch (IllegalArgumentException e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}
