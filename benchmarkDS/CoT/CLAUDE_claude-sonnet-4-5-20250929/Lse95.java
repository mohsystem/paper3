
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.sql.*;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse95 {
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_LENGTH = 16;
    
    public static String[] registerUser(String username, String password) throws Exception {
        byte[] salt = generateSalt();
        byte[] hash = hashPassword(password, salt);
        
        String saltBase64 = Base64.getEncoder().encodeToString(salt);
        String hashBase64 = Base64.getEncoder().encodeToString(hash);
        String storedPassword = saltBase64 + ":" + hashBase64;
        
        return new String[]{username, storedPassword};
    }
    
    public static boolean verifyPassword(String password, String storedPassword) throws Exception {
        String[] parts = storedPassword.split(":");
        if (parts.length != 2) {
            return false;
        }
        
        byte[] salt = Base64.getDecoder().decode(parts[0]);
        byte[] storedHash = Base64.getDecoder().decode(parts[1]);
        byte[] computedHash = hashPassword(password, salt);
        
        return slowEquals(storedHash, computedHash);
    }
    
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    private static byte[] hashPassword(String password, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return factory.generateSecret(spec).getEncoded();
    }
    
    private static boolean slowEquals(byte[] a, byte[] b) {
        int diff = a.length ^ b.length;
        for (int i = 0; i < a.length && i < b.length; i++) {
            diff |= a[i] ^ b[i];
        }
        return diff == 0;
    }
    
    public static void main(String[] args) {
        try {
            // Test Case 1: Basic registration
            String[] user1 = registerUser("alice", "SecurePass123!");
            System.out.println("Test 1 - Registered: " + user1[0]);
            System.out.println("Verification: " + verifyPassword("SecurePass123!", user1[1]));
            
            // Test Case 2: Different user
            String[] user2 = registerUser("bob", "MyPassword456@");
            System.out.println("\\nTest 2 - Registered: " + user2[0]);
            System.out.println("Verification: " + verifyPassword("MyPassword456@", user2[1]));
            
            // Test Case 3: Wrong password
            System.out.println("\\nTest 3 - Wrong password: " + verifyPassword("WrongPass", user1[1]));
            
            // Test Case 4: Complex password
            String[] user3 = registerUser("charlie", "C0mpl3x!P@ssw0rd#2024");
            System.out.println("\\nTest 4 - Registered: " + user3[0]);
            System.out.println("Verification: " + verifyPassword("C0mpl3x!P@ssw0rd#2024", user3[1]));
            
            // Test Case 5: Empty username with valid password
            String[] user4 = registerUser("david", "AnotherSecure987$");
            System.out.println("\\nTest 5 - Registered: " + user4[0]);
            System.out.println("Verification: " + verifyPassword("AnotherSecure987$", user4[1]));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
