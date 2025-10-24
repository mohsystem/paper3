
import java.io.*;
import java.util.Scanner;

public class Task89 {
    public static boolean comparePasswords(String storedPassword, String enteredPassword) {
        if (storedPassword == null || enteredPassword == null) {
            return false;
        }
        
        if (storedPassword.length() != enteredPassword.length()) {
            return false;
        }
        
        for (int i = 0; i < storedPassword.length(); i++) {
            if (storedPassword.charAt(i) != enteredPassword.charAt(i)) {
                return false;
            }
        }
        
        return true;
    }
    
    public static String readPasswordFromFile(String filename) {
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            return reader.readLine();
        } catch (IOException e) {
            System.err.println("Error reading password file: " + e.getMessage());
            return null;
        }
    }
    
    public static boolean verifyPassword(String filename, String enteredPassword) {
        String storedPassword = readPasswordFromFile(filename);
        return comparePasswords(storedPassword, enteredPassword);
    }
    
    public static void main(String[] args) {
        // Create a test password file
        String testFile = "password.txt";
        try (PrintWriter writer = new PrintWriter(new FileWriter(testFile))) {
            writer.println("SecurePass123");
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
        }
        
        // Test cases
        System.out.println("Test Case 1 - Correct password:");
        boolean result1 = verifyPassword(testFile, "SecurePass123");
        System.out.println("Result: " + (result1 ? "Access Granted" : "Access Denied"));
        
        System.out.println("\\nTest Case 2 - Incorrect password:");
        boolean result2 = verifyPassword(testFile, "WrongPass456");
        System.out.println("Result: " + (result2 ? "Access Granted" : "Access Denied"));
        
        System.out.println("\\nTest Case 3 - Empty password:");
        boolean result3 = verifyPassword(testFile, "");
        System.out.println("Result: " + (result3 ? "Access Granted" : "Access Denied"));
        
        System.out.println("\\nTest Case 4 - Case sensitive check:");
        boolean result4 = verifyPassword(testFile, "securepass123");
        System.out.println("Result: " + (result4 ? "Access Granted" : "Access Denied"));
        
        System.out.println("\\nTest Case 5 - Partial match:");
        boolean result5 = verifyPassword(testFile, "SecurePass");
        System.out.println("Result: " + (result5 ? "Access Granted" : "Access Denied"));
        
        // Clean up test file
        new File(testFile).delete();
    }
}
