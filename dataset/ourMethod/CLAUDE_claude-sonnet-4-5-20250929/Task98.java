
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Task98 {
    private static final int MAX_INPUT_LENGTH = 100;
    private static final int MIN_INT_VALUE = Integer.MIN_VALUE;
    private static final int MAX_INT_VALUE = Integer.MAX_VALUE;

    public static Integer convertToInteger(String input) {
        if (input == null) {
            return null;
        }
        
        String trimmed = input.trim();
        
        if (trimmed.isEmpty() || trimmed.length() > MAX_INPUT_LENGTH) {
            return null;
        }
        
        // Validate characters: allow optional leading sign and digits only
        if (!trimmed.matches("^[+-]?\\\\d+$")) {
            return null;
        }
        
        try {
            long parsedValue = Long.parseLong(trimmed);
            
            if (parsedValue < MIN_INT_VALUE || parsedValue > MAX_INT_VALUE) {
                return null;
            }
            
            return (int) parsedValue;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "42",
            "-123",
            "0",
            "2147483647",
            "invalid123"
        };
        
        System.out.println("Running test cases:");
        for (String testCase : testCases) {
            Integer result = convertToInteger(testCase);
            System.out.println("Input: \\"" + testCase + "\\" -> Output: " + result);
        }
        
        // Interactive console input
        System.out.println("\\nEnter an integer to convert:");
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String input = reader.readLine();
            
            if (input != null && input.length() <= MAX_INPUT_LENGTH) {
                Integer result = convertToInteger(input);
                if (result != null) {
                    System.out.println("Converted integer: " + result);
                } else {
                    System.out.println("Error: Invalid integer format");
                }
            } else {
                System.out.println("Error: Input exceeds maximum length");
            }
        } catch (IOException e) {
            System.err.println("Error reading input");
        }
    }
}
