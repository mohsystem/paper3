
import java.util.Scanner;

public class Task41 {
    private static final int MAX_LENGTH = 1024;
    
    public static String processString(String input) {
        if (input == null) {
            return "Error: Null input";
        }
        
        if (input.length() > MAX_LENGTH) {
            return "Error: Input exceeds maximum length of " + MAX_LENGTH;
        }
        
        // Safe string processing
        StringBuilder processed = new StringBuilder();
        processed.append("Processed: ");
        processed.append(input.trim());
        processed.append(" (Length: ").append(input.length()).append(")");
        
        return processed.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal input
        System.out.println("Test 1: " + processString("Hello World"));
        
        // Test case 2: Empty string
        System.out.println("Test 2: " + processString(""));
        
        // Test case 3: String with whitespace
        System.out.println("Test 3: " + processString("  Test String  "));
        
        // Test case 4: Null input
        System.out.println("Test 4: " + processString(null));
        
        // Test case 5: Long string
        String longString = "A".repeat(500);
        System.out.println("Test 5: " + processString(longString));
        
        // Interactive mode
        Scanner scanner = new Scanner(System.in);
        System.out.println("\\nEnter a string to process (or 'exit' to quit):");
        while (scanner.hasNextLine()) {
            String input = scanner.nextLine();
            if (input.equals("exit")) {
                break;
            }
            System.out.println(processString(input));
            System.out.println("Enter another string (or 'exit' to quit):");
        }
        scanner.close();
    }
}
