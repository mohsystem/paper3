
import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class Task41 {
    private static final int MAX_INPUT_LENGTH = 1024;
    
    public static String processString(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length of " + MAX_INPUT_LENGTH);
        }
        
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        if (bytes.length > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input byte size exceeds maximum length");
        }
        
        StringBuilder processed = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (Character.isLetterOrDigit(c) || Character.isWhitespace(c) || 
                c == '.' || c == ',' || c == '!' || c == '?') {
                processed.append(c);
            }
        }
        
        return processed.toString();
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "Hello World",
            "Test123 with numbers!",
            "Special chars: @#$%^&*()",
            "A".repeat(100),
            ""
        };
        
        System.out.println("Running test cases:");
        for (int i = 0; i < testCases.length; i++) {
            try {
                String result = processString(testCases[i]);
                System.out.println("Test " + (i + 1) + ": Success");
                System.out.println("  Input: " + testCases[i].substring(0, Math.min(50, testCases[i].length())));
                System.out.println("  Output: " + result.substring(0, Math.min(50, result.length())));
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + ": Error - " + e.getMessage());
            }
        }
    }
}
