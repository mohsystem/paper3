
import java.util.Scanner;

public class Task98 {
    public static Integer convertToInteger(String input) {
        if (input == null || input.trim().isEmpty()) {
            return null;
        }
        
        try {
            return Integer.parseInt(input.trim());
        } catch (NumberFormatException e) {
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "abc", "2147483648"};
        
        System.out.println("Running test cases:");
        for (String testCase : testCases) {
            Integer result = convertToInteger(testCase);
            if (result != null) {
                System.out.println("Input: '" + testCase + "' -> Output: " + result);
            } else {
                System.out.println("Input: '" + testCase + "' -> Invalid input");
            }
        }
        
        // Interactive mode
        System.out.println("\\n--- Interactive Mode ---");
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter a value to convert to integer: ");
        
        if (scanner.hasNextLine()) {
            String userInput = scanner.nextLine();
            Integer result = convertToInteger(userInput);
            
            if (result != null) {
                System.out.println("Successfully converted to integer: " + result);
            } else {
                System.out.println("Error: Invalid input. Please enter a valid integer.");
            }
        }
        
        scanner.close();
    }
}
