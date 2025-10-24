
import java.util.*;
import java.io.*;

public class Task45 {
    public static String performOperation(String operation, double num1, double num2) {
        try {
            switch (operation.toLowerCase()) {
                case "add":
                    return String.valueOf(num1 + num2);
                case "subtract":
                    return String.valueOf(num1 - num2);
                case "multiply":
                    return String.valueOf(num1 * num2);
                case "divide":
                    if (num2 == 0) {
                        throw new ArithmeticException("Division by zero");
                    }
                    return String.valueOf(num1 / num2);
                case "modulo":
                    if (num2 == 0) {
                        throw new ArithmeticException("Modulo by zero");
                    }
                    return String.valueOf(num1 % num2);
                case "power":
                    return String.valueOf(Math.pow(num1, num2));
                default:
                    throw new IllegalArgumentException("Invalid operation: " + operation);
            }
        } catch (ArithmeticException e) {
            return "Error: " + e.getMessage();
        } catch (IllegalArgumentException e) {
            return "Error: " + e.getMessage();
        } catch (Exception e) {
            return "Error: Unexpected error occurred - " + e.getMessage();
        }
    }
    
    public static String parseAndCalculate(String input) {
        try {
            if (input == null || input.trim().isEmpty()) {
                throw new IllegalArgumentException("Input cannot be empty");
            }
            
            String[] parts = input.trim().split("\\\\s+");
            
            if (parts.length != 3) {
                throw new IllegalArgumentException("Invalid input format. Expected: <operation> <num1> <num2>");
            }
            
            String operation = parts[0];
            double num1 = Double.parseDouble(parts[1]);
            double num2 = Double.parseDouble(parts[2]);
            
            return performOperation(operation, num1, num2);
            
        } catch (NumberFormatException e) {
            return "Error: Invalid number format - " + e.getMessage();
        } catch (IllegalArgumentException e) {
            return "Error: " + e.getMessage();
        } catch (Exception e) {
            return "Error: Unexpected error - " + e.getMessage();
        }
    }
    
    public static String validateAndProcess(String[] inputs) {
        try {
            if (inputs == null || inputs.length == 0) {
                throw new NullPointerException("Input array is null or empty");
            }
            
            StringBuilder results = new StringBuilder();
            for (int i = 0; i < inputs.length; i++) {
                try {
                    String result = parseAndCalculate(inputs[i]);
                    results.append("Input ").append(i + 1).append(": ").append(result).append("\\n");
                } catch (Exception e) {
                    results.append("Input ").append(i + 1).append(": Error - ").append(e.getMessage()).append("\\n");
                }
            }
            return results.toString();
            
        } catch (NullPointerException e) {
            return "Error: " + e.getMessage();
        } catch (Exception e) {
            return "Error: Unexpected error in batch processing - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid Addition ===");
        System.out.println(parseAndCalculate("add 10 5"));
        
        System.out.println("\\n=== Test Case 2: Division by Zero ===");
        System.out.println(parseAndCalculate("divide 10 0"));
        
        System.out.println("\\n=== Test Case 3: Invalid Operation ===");
        System.out.println(parseAndCalculate("invalid 10 5"));
        
        System.out.println("\\n=== Test Case 4: Invalid Number Format ===");
        System.out.println(parseAndCalculate("multiply abc 5"));
        
        System.out.println("\\n=== Test Case 5: Batch Processing ===");
        String[] batchInputs = {
            "add 100 50",
            "subtract 75 25",
            "multiply 8 7",
            "divide 100 4",
            "power 2 8"
        };
        System.out.println(validateAndProcess(batchInputs));
    }
}
