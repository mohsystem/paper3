
import java.util.Scanner;

public class Task41 {
    public static String processString(String input) {
        if (input == null) {
            return "Error: Input is null";
        }
        
        // Java strings are immutable and automatically managed
        // No buffer overflow possible
        return "Processed: " + input;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1: " + processString("Hello World"));
        
        // Test case 2: Empty string
        System.out.println("Test 2: " + processString(""));
        
        // Test case 3: Very long string
        StringBuilder longString = new StringBuilder();
        for (int i = 0; i < 10000; i++) {
            longString.append("a");
        }
        System.out.println("Test 3: " + processString(longString.toString()).substring(0, 50) + "...");
        
        // Test case 4: Special characters
        System.out.println("Test 4: " + processString("!@#$%^&*()_+-=[]{}|;':,.<>?"));
        
        // Test case 5: Null input
        System.out.println("Test 5: " + processString(null));
    }
}
