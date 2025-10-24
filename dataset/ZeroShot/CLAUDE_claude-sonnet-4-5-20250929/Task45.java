
import java.util.*;
import java.io.*;

public class Task45 {
    
    public static String performOperation(String input, String operation) {
        try {
            if (input == null || operation == null) {
                throw new IllegalArgumentException("Input and operation cannot be null");
            }
            
            input = input.trim();
            operation = operation.trim().toLowerCase();
            
            switch (operation) {
                case "uppercase":
                    return input.toUpperCase();
                    
                case "lowercase":
                    return input.toLowerCase();
                    
                case "reverse":
                    return new StringBuilder(input).reverse().toString();
                    
                case "length":
                    return String.valueOf(input.length());
                    
                case "wordcount":
                    if (input.isEmpty()) return "0";
                    return String.valueOf(input.split("\\\\s+").length);
                    
                case "number":
                    try {
                        double num = Double.parseDouble(input);
                        return String.valueOf(num * 2);
                    } catch (NumberFormatException e) {
                        throw new IllegalArgumentException("Input is not a valid number for 'number' operation");
                    }
                    
                default:
                    throw new IllegalArgumentException("Unknown operation: " + operation);
            }
        } catch (IllegalArgumentException e) {
            return "Error: " + e.getMessage();
        } catch (Exception e) {
            return "Error: Unexpected error occurred - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Cases ===\\n");
        
        // Test case 1: Uppercase operation
        String result1 = performOperation("hello world", "uppercase");
        System.out.println("Test 1 - Uppercase: " + result1);
        
        // Test case 2: Reverse operation
        String result2 = performOperation("Java Programming", "reverse");
        System.out.println("Test 2 - Reverse: " + result2);
        
        // Test case 3: Word count operation
        String result3 = performOperation("This is a test sentence", "wordcount");
        System.out.println("Test 3 - Word Count: " + result3);
        
        // Test case 4: Number operation with valid input
        String result4 = performOperation("42.5", "number");
        System.out.println("Test 4 - Number Operation: " + result4);
        
        // Test case 5: Error handling - invalid operation
        String result5 = performOperation("test", "invalid_op");
        System.out.println("Test 5 - Invalid Operation: " + result5);
    }
}
