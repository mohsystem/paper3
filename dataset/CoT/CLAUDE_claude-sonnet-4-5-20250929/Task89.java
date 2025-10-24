
import java.io.*;
import java.nio.file.*;
import java.util.Scanner;

public class Task89 {
    
    // Constant-time string comparison to prevent timing attacks
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        
        int lengthA = a.length();
        int lengthB = b.length();
        
        // Use the longer length to prevent timing attacks
        int maxLength = Math.max(lengthA, lengthB);
        int result = lengthA ^ lengthB; // Different lengths contribute to mismatch
        
        for (int i = 0; i < maxLength; i++) {
            char charA = (i < lengthA) ? a.charAt(i) : 0;
            char charB = (i < lengthB) ? b.charAt(i) : 0;
            result |= charA ^ charB;
        }
        
        return result == 0;
    }
    
    public static boolean verifyPassword(String filePath, String enteredPassword) {
        String storedPassword = null;
        
        try {
            // Read password from file securely
            storedPassword = new String(Files.readAllBytes(Paths.get(filePath))).trim();
            
            // Constant-time comparison
            boolean matches = constantTimeEquals(storedPassword, enteredPassword);
            
            return matches;
            
        } catch (IOException e) {
            System.err.println("Error reading password file: " + e.getMessage());
            return false;
        } finally {
            // Clear sensitive data from memory
            if (storedPassword != null) {
                storedPassword = null;
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Password Verification Test Cases ===\\n");
        
        // Create test password file
        String testFile = "test_password.txt";
        try {
            Files.write(Paths.get(testFile), "SecurePass123!".getBytes());
        } catch (IOException e) {
            System.err.println("Failed to create test file");
            return;
        }
        
        // Test Case 1: Correct password
        System.out.println("Test 1 - Correct password:");
        boolean result1 = verifyPassword(testFile, "SecurePass123!");
        System.out.println("Result: " + (result1 ? "MATCH" : "NO MATCH") + "\\n");
        
        // Test Case 2: Incorrect password
        System.out.println("Test 2 - Incorrect password:");
        boolean result2 = verifyPassword(testFile, "WrongPassword");
        System.out.println("Result: " + (result2 ? "MATCH" : "NO MATCH") + "\\n");
        
        // Test Case 3: Password with different case
        System.out.println("Test 3 - Different case:");
        boolean result3 = verifyPassword(testFile, "securepass123!");
        System.out.println("Result: " + (result3 ? "MATCH" : "NO MATCH") + "\\n");
        
        // Test Case 4: Empty password
        System.out.println("Test 4 - Empty password:");
        boolean result4 = verifyPassword(testFile, "");
        System.out.println("Result: " + (result4 ? "MATCH" : "NO MATCH") + "\\n");
        
        // Test Case 5: Password with extra characters
        System.out.println("Test 5 - Password with extra characters:");
        boolean result5 = verifyPassword(testFile, "SecurePass123!Extra");
        System.out.println("Result: " + (result5 ? "MATCH" : "NO MATCH") + "\\n");
        
        // Cleanup test file
        try {
            Files.deleteIfExists(Paths.get(testFile));
        } catch (IOException e) {
            System.err.println("Failed to delete test file");
        }
    }
}
