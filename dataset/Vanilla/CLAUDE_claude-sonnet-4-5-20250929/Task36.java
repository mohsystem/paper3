
import java.io.*;
import java.util.*;

public class Task36 {
    public static String readFileContents(String filename) {
        StringBuilder content = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\\n");
            }
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
        return content.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Create and read a simple text file
        try {
            PrintWriter writer = new PrintWriter("test1.txt");
            writer.println("Hello, World!");
            writer.println("This is test file 1.");
            writer.close();
            System.out.println("Test Case 1:");
            System.out.println(readFileContents("test1.txt"));
        } catch (Exception e) {
            System.out.println("Error in test case 1: " + e.getMessage());
        }
        
        // Test case 2: Create and read a file with multiple lines
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("Line 1");
            writer.println("Line 2");
            writer.println("Line 3");
            writer.close();
            System.out.println("Test Case 2:");
            System.out.println(readFileContents("test2.txt"));
        } catch (Exception e) {
            System.out.println("Error in test case 2: " + e.getMessage());
        }
        
        // Test case 3: Empty file
        try {
            PrintWriter writer = new PrintWriter("test3.txt");
            writer.close();
            System.out.println("Test Case 3 (Empty file):");
            System.out.println(readFileContents("test3.txt"));
        } catch (Exception e) {
            System.out.println("Error in test case 3: " + e.getMessage());
        }
        
        // Test case 4: File with special characters
        try {
            PrintWriter writer = new PrintWriter("test4.txt");
            writer.println("Special chars: @#$%^&*()");
            writer.println("Numbers: 123456789");
            writer.close();
            System.out.println("Test Case 4:");
            System.out.println(readFileContents("test4.txt"));
        } catch (Exception e) {
            System.out.println("Error in test case 4: " + e.getMessage());
        }
        
        // Test case 5: Non-existent file
        System.out.println("Test Case 5 (Non-existent file):");
        System.out.println(readFileContents("nonexistent.txt"));
    }
}
