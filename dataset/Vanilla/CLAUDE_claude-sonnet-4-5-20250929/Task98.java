
import java.util.Scanner;

public class Task98 {
    public static Integer convertToInteger(String input) {
        try {
            return Integer.parseInt(input.trim());
        } catch (NumberFormatException e) {
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "abc", "12.34"};
        
        System.out.println("Running test cases:");
        for (String testCase : testCases) {
            Integer result = convertToInteger(testCase);
            if (result != null) {
                System.out.println("Input: '" + testCase + "' -> Output: " + result);
            } else {
                System.out.println("Input: '" + testCase + "' -> Output: Invalid (cannot convert to integer)");
            }
        }
        
        // Interactive console input
        System.out.println("\\nEnter a value to convert to integer:");
        Scanner scanner = new Scanner(System.in);
        String userInput = scanner.nextLine();
        Integer result = convertToInteger(userInput);
        
        if (result != null) {
            System.out.println("Successfully converted to integer: " + result);
        } else {
            System.out.println("Error: Cannot convert '" + userInput + "' to integer");
        }
        
        scanner.close();
    }
}
