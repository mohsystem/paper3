
import java.util.Scanner;
import java.util.ArrayList;
import java.util.List;

public class Task113 {
    // Method to safely concatenate strings with input validation
    public static String concatenateStrings(List<String> strings) {
        if (strings == null || strings.isEmpty()) {
            return "";
        }
        
        // Using StringBuilder for efficient string concatenation
        StringBuilder result = new StringBuilder();
        
        for (String str : strings) {
            if (str != null) {
                // Sanitize input to prevent injection attacks
                String sanitized = sanitizeInput(str);
                result.append(sanitized);
            }
        }
        
        return result.toString();
    }
    
    // Method to sanitize input strings
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Remove any potential control characters while preserving valid input
        return input.replaceAll("[\\\\x00-\\\\x08\\\\x0B\\\\x0C\\\\x0E-\\\\x1F\\\\x7F]", "");
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("=== Test Cases ===\\n");
        
        // Test Case 1: Normal strings
        List<String> test1 = new ArrayList<>();
        test1.add("Hello");
        test1.add(" ");
        test1.add("World");
        test1.add("!");
        System.out.println("Test 1 - Normal strings: " + concatenateStrings(test1));
        
        // Test Case 2: Empty list
        List<String> test2 = new ArrayList<>();
        System.out.println("Test 2 - Empty list: '" + concatenateStrings(test2) + "'");
        
        // Test Case 3: Strings with numbers and special characters
        List<String> test3 = new ArrayList<>();
        test3.add("User");
        test3.add("123");
        test3.add("@");
        test3.add("Example");
        System.out.println("Test 3 - Mixed content: " + concatenateStrings(test3));
        
        // Test Case 4: List with null values
        List<String> test4 = new ArrayList<>();
        test4.add("Start");
        test4.add(null);
        test4.add("End");
        System.out.println("Test 4 - With null values: " + concatenateStrings(test4));
        
        // Test Case 5: Single string
        List<String> test5 = new ArrayList<>();
        test5.add("SingleString");
        System.out.println("Test 5 - Single string: " + concatenateStrings(test5));
        
        // Interactive input option
        System.out.println("\\n=== Interactive Mode ===");
        Scanner scanner = new Scanner(System.in);
        List<String> userInputs = new ArrayList<>();
        
        System.out.println("Enter number of strings to concatenate:");
        try {
            int count = Integer.parseInt(scanner.nextLine().trim());
            
            if (count < 0 || count > 1000) {
                System.out.println("Invalid count. Please enter a value between 0 and 1000.");
                return;
            }
            
            for (int i = 0; i < count; i++) {
                System.out.println("Enter string " + (i + 1) + ":");
                String input = scanner.nextLine();
                userInputs.add(input);
            }
            
            String result = concatenateStrings(userInputs);
            System.out.println("\\nConcatenated Result: " + result);
            
        } catch (NumberFormatException e) {
            System.out.println("Invalid input. Please enter a valid number.");
        } finally {
            scanner.close();
        }
    }
}
