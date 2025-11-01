
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Scanner;

public class Lse92 {
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashedBytes = md.digest(password.getBytes());
            StringBuilder sb = new StringBuilder();
            for (byte b : hashedBytes) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void registerUser(String username, String password) {
        String hashedPassword = hashPassword(password);
        System.out.println("Username: " + username);
        System.out.println("Hashed Password: " + hashedPassword);
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        registerUser("john_doe", "password123");
        System.out.println();
        
        System.out.println("Test Case 2:");
        registerUser("alice", "securePass!@#");
        System.out.println();
        
        System.out.println("Test Case 3:");
        registerUser("bob_smith", "myP@ssw0rd");
        System.out.println();
        
        System.out.println("Test Case 4:");
        registerUser("user123", "12345678");
        System.out.println();
        
        System.out.println("Test Case 5:");
        registerUser("admin", "adminPass2024");
        System.out.println();
    }
}
