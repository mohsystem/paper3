
import java.util.*;

public class Task51 {
    public static String encrypt(String input) {
        if (input == null || input.isEmpty()) {
            return input;
        }
        
        StringBuilder encrypted = new StringBuilder();
        int shift = 3; // Caesar cipher shift
        
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            
            if (Character.isUpperCase(ch)) {
                char encryptedChar = (char) ((ch - 'A' + shift) % 26 + 'A');
                encrypted.append(encryptedChar);
            } else if (Character.isLowerCase(ch)) {
                char encryptedChar = (char) ((ch - 'a' + shift) % 26 + 'a');
                encrypted.append(encryptedChar);
            } else if (Character.isDigit(ch)) {
                char encryptedChar = (char) ((ch - '0' + shift) % 10 + '0');
                encrypted.append(encryptedChar);
            } else {
                encrypted.append(ch);
            }
        }
        
        return encrypted.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple lowercase string
        String test1 = "hello";
        System.out.println("Test 1 - Input: " + test1);
        System.out.println("Encrypted: " + encrypt(test1));
        System.out.println();
        
        // Test case 2: Mixed case string
        String test2 = "HelloWorld";
        System.out.println("Test 2 - Input: " + test2);
        System.out.println("Encrypted: " + encrypt(test2));
        System.out.println();
        
        // Test case 3: String with numbers
        String test3 = "Test123";
        System.out.println("Test 3 - Input: " + test3);
        System.out.println("Encrypted: " + encrypt(test3));
        System.out.println();
        
        // Test case 4: String with special characters
        String test4 = "Hello, World! 2024";
        System.out.println("Test 4 - Input: " + test4);
        System.out.println("Encrypted: " + encrypt(test4));
        System.out.println();
        
        // Test case 5: Edge case with letters near end of alphabet
        String test5 = "xyz XYZ 789";
        System.out.println("Test 5 - Input: " + test5);
        System.out.println("Encrypted: " + encrypt(test5));
        System.out.println();
    }
}
