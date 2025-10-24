
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
        try {
            BufferedReader reader = new BufferedReader(new FileReader(filename));
            String password = reader.readLine();
            reader.close();
            return password;
        } catch (IOException e) {
            return null;
        }
    }
    
    public static boolean verifyPassword(String filename, String enteredPassword) {
        String storedPassword = readPasswordFromFile(filename);
        return comparePasswords(storedPassword, enteredPassword);
    }
    
    public static void main(String[] args) {
        // Create test file with password
        try {
            PrintWriter writer = new PrintWriter(new FileWriter("password.txt"));
            writer.println("SecurePass123");
            writer.close();
        } catch (IOException e) {
            System.out.println("Error creating test file");
        }
        
        // Test Case 1: Correct password
        System.out.println("Test 1 - Correct password:");
        boolean result1 = verifyPassword("password.txt", "SecurePass123");
        System.out.println("Result: " + result1 + " (Expected: true)\\n");
        
        // Test Case 2: Incorrect password
        System.out.println("Test 2 - Incorrect password:");
        boolean result2 = verifyPassword("password.txt", "WrongPass123");
        System.out.println("Result: " + result2 + " (Expected: false)\\n");
        
        // Test Case 3: Password with different length
        System.out.println("Test 3 - Different length:");
        boolean result3 = verifyPassword("password.txt", "Short");
        System.out.println("Result: " + result3 + " (Expected: false)\\n");
        
        // Test Case 4: Empty password
        System.out.println("Test 4 - Empty password:");
        boolean result4 = verifyPassword("password.txt", "");
        System.out.println("Result: " + result4 + " (Expected: false)\\n");
        
        // Test Case 5: Case sensitive check
        System.out.println("Test 5 - Case sensitive:");
        boolean result5 = verifyPassword("password.txt", "securepass123");
        System.out.println("Result: " + result5 + " (Expected: false)\\n");
    }
}
