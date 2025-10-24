
import java.util.Scanner;
import java.util.Arrays;

public class Task104 {
    private static final int BUFFER_SIZE = 100;
    
    public static String handleUserInput(String input) {
        if (input == null) {
            return "";
        }
        
        // Truncate input if it exceeds buffer size
        if (input.length() > BUFFER_SIZE) {
            return input.substring(0, BUFFER_SIZE);
        }
        
        return input;
    }
    
    public static char[] handleUserInputToCharArray(String input) {
        char[] buffer = new char[BUFFER_SIZE];
        Arrays.fill(buffer, '\\0');
        
        if (input == null) {
            return buffer;
        }
        
        int length = Math.min(input.length(), BUFFER_SIZE);
        for (int i = 0; i < length; i++) {
            buffer[i] = input.charAt(i);
        }
        
        return buffer;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Normal input");
        String input1 = "Hello, World!";
        String result1 = handleUserInput(input1);
        System.out.println("Input: " + input1);
        System.out.println("Output: " + result1);
        System.out.println("Length: " + result1.length());
        System.out.println();
        
        System.out.println("Test Case 2: Empty input");
        String input2 = "";
        String result2 = handleUserInput(input2);
        System.out.println("Input: (empty)");
        System.out.println("Output: " + result2);
        System.out.println("Length: " + result2.length());
        System.out.println();
        
        System.out.println("Test Case 3: Input exceeding buffer size");
        String input3 = "A".repeat(150);
        String result3 = handleUserInput(input3);
        System.out.println("Input length: " + input3.length());
        System.out.println("Output length: " + result3.length());
        System.out.println("Truncated: " + (result3.length() == BUFFER_SIZE));
        System.out.println();
        
        System.out.println("Test Case 4: Input at buffer boundary");
        String input4 = "B".repeat(100);
        String result4 = handleUserInput(input4);
        System.out.println("Input length: " + input4.length());
        System.out.println("Output length: " + result4.length());
        System.out.println();
        
        System.out.println("Test Case 5: Null input");
        String input5 = null;
        String result5 = handleUserInput(input5);
        System.out.println("Input: null");
        System.out.println("Output: " + result5);
        System.out.println("Length: " + result5.length());
    }
}
