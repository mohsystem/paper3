
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task44 {
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final Pattern ALPHANUMERIC_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s]+$");
    private static final Pattern NUMERIC_PATTERN = Pattern.compile("^-?\\\\d+$");
    
    public static String validateAndProcessString(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() == 0) {
            throw new IllegalArgumentException("Input cannot be empty");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length of " + MAX_INPUT_LENGTH);
        }
        
        if (!ALPHANUMERIC_PATTERN.matcher(input).matches()) {
            throw new IllegalArgumentException("Input contains invalid characters. Only alphanumeric characters and spaces are allowed");
        }
        
        String trimmed = input.trim();
        String result = trimmed.toUpperCase();
        return result;
    }
    
    public static int validateAndProcessNumber(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        String trimmed = input.trim();
        
        if (trimmed.length() == 0) {
            throw new IllegalArgumentException("Input cannot be empty");
        }
        
        if (!NUMERIC_PATTERN.matcher(trimmed).matches()) {
            throw new IllegalArgumentException("Input is not a valid integer");
        }
        
        try {
            int number = Integer.parseInt(trimmed);
            
            if (number < -1000000 || number > 1000000) {
                throw new IllegalArgumentException("Number must be between -1000000 and 1000000");
            }
            
            return number * 2;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Input is not a valid integer or out of range");
        }
    }
    
    public static String concatenateValidatedStrings(String input1, String input2) {
        String validated1 = validateAndProcessString(input1);
        String validated2 = validateAndProcessString(input2);
        
        if (validated1.length() + validated2.length() + 1 > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Combined length exceeds maximum allowed");
        }
        
        return validated1 + " " + validated2;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid alphanumeric string");
        try {
            String result1 = validateAndProcessString("Hello World 123");
            System.out.println("Result: " + result1);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 2: Valid number");
        try {
            int result2 = validateAndProcessNumber("42");
            System.out.println("Result: " + result2);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Invalid string with special characters");
        try {
            String result3 = validateAndProcessString("Hello@World!");
            System.out.println("Result: " + result3);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Invalid number");
        try {
            int result4 = validateAndProcessNumber("not a number");
            System.out.println("Result: " + result4);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 5: Concatenate two valid strings");
        try {
            String result5 = concatenateValidatedStrings("Test 123", "Data 456");
            System.out.println("Result: " + result5);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
