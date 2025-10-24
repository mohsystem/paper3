
import java.io.*;
import java.util.Scanner;

public class Task158 {
    public static String executeCommand(String userInput) {
        StringBuilder output = new StringBuilder();
        try {
            // Vulnerable: Direct use of user input in OS command
            String command = "echo " + userInput;
            Process process = Runtime.getRuntime().exec(command);
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
        return output.toString().trim();
    }
    
    public static void main(String[] args) {
        System.out.println("=== OS Command Injection Demo ===\\n");
        
        // Test case 1: Normal input
        System.out.println("Test 1 - Normal input:");
        System.out.println("Input: 'Hello World'");
        System.out.println("Output: " + executeCommand("Hello World"));
        System.out.println();
        
        // Test case 2: Simple text
        System.out.println("Test 2 - Simple text:");
        System.out.println("Input: 'Testing123'");
        System.out.println("Output: " + executeCommand("Testing123"));
        System.out.println();
        
        // Test case 3: Number input
        System.out.println("Test 3 - Number input:");
        System.out.println("Input: '42'");
        System.out.println("Output: " + executeCommand("42"));
        System.out.println();
        
        // Test case 4: Special characters
        System.out.println("Test 4 - Special characters:");
        System.out.println("Input: 'test@example.com'");
        System.out.println("Output: " + executeCommand("test@example.com"));
        System.out.println();
        
        // Test case 5: Multiple words
        System.out.println("Test 5 - Multiple words:");
        System.out.println("Input: 'This is a test'");
        System.out.println("Output: " + executeCommand("This is a test"));
        System.out.println();
    }
}
