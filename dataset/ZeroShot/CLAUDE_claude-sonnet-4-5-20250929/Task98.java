
import java.util.Scanner;
import java.util.InputMismatchException;

public class Task98 {
    public static Integer convertToInteger(String input) {
        try {
            return Integer.parseInt(input.trim());
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid input. Cannot convert '" + input + "' to integer.");
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "abc", "12.34"};
        
        System.out.println("Running test cases:");
        for (String testCase : testCases) {
            System.out.print("Input: '" + testCase + "' -> ");
            Integer result = convertToInteger(testCase);
            if (result != null) {
                System.out.println("Output: " + result);
            }
        }
        
        // Interactive input
        System.out.println("\\nEnter a value to convert to integer (or 'exit' to quit):");
        Scanner scanner = new Scanner(System.in);
        
        while (scanner.hasNextLine()) {
            String input = scanner.nextLine();
            if (input.equalsIgnoreCase("exit")) {
                break;
            }
            Integer result = convertToInteger(input);
            if (result != null) {
                System.out.println("Successfully converted to: " + result);
            }
            System.out.println("Enter another value (or 'exit' to quit):");
        }
        
        scanner.close();
    }
}
