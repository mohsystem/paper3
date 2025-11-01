
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse96 {
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static String registerUser(String username, String password) {
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        String storedValue = salt + ":" + hashedPassword;
        return storedValue;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String result1 = registerUser("user1", "password123");
        System.out.println("User1 registration: " + result1);
        
        // Test case 2
        String result2 = registerUser("alice", "securePass!@#");
        System.out.println("Alice registration: " + result2);
        
        // Test case 3
        String result3 = registerUser("bob", "mySecret");
        System.out.println("Bob registration: " + result3);
        
        // Test case 4
        String result4 = registerUser("admin", "Admin@2024");
        System.out.println("Admin registration: " + result4);
        
        // Test case 5
        String result5 = registerUser("test_user", "Test1234");
        System.out.println("Test_user registration: " + result5);
    }
}
