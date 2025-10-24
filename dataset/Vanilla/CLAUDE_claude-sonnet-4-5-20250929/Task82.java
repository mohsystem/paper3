
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Task82 {
    public static String hashPassword(String password, String salt) {
        try {
            String combined = password + salt;
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hashBytes = digest.digest(combined.getBytes(StandardCharsets.UTF_8));
            
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String hash1 = hashPassword("password123", "randomsalt");
        System.out.println("Test 1: " + hash1);
        
        // Test case 2
        String hash2 = hashPassword("mySecurePass", "salt456");
        System.out.println("Test 2: " + hash2);
        
        // Test case 3
        String hash3 = hashPassword("admin", "abcdef");
        System.out.println("Test 3: " + hash3);
        
        // Test case 4
        String hash4 = hashPassword("test", "xyz");
        System.out.println("Test 4: " + hash4);
        
        // Test case 5
        String hash5 = hashPassword("", "emptysalt");
        System.out.println("Test 5: " + hash5);
    }
}
