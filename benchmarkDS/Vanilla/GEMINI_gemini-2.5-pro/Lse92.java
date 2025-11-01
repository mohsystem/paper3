import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse92 {

    /**
     * Hashes a password using the SHA-256 algorithm.
     * @param username The username (not used in hashing, but part of a typical registration).
     * @param password The password to hash.
     * @return The SHA-256 hashed password as a hexadecimal string, or null on error.
     */
    public static String registerUser(String username, String password) {
        try {
            // Create a MessageDigest instance for SHA-256
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            
            // Get the hash's bytes
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            
            // Convert the byte array into a hexadecimal string
            StringBuilder hexString = new StringBuilder(2 * hash.length);
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            // This error should not happen for a standard algorithm like SHA-256
            System.err.println("SHA-256 algorithm not found.");
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        String[][] testCases = {
            {"admin", "password123"},
            {"user", "P@ssw0rd!"},
            {"guest", ""},
            {"test_user_long_name", "a_very_long_and_secure_password_with_numbers_12345_and_symbols_!@#$"},
            {"another_user", "password123"}
        };
        
        System.out.println("--- Java Test Cases ---");
        for (String[] testCase : testCases) {
            String username = testCase[0];
            String password = testCase[1];
            String hashedPassword = registerUser(username, password);
            System.out.println("Username: " + username);
            System.out.println("Password: " + password);
            System.out.println("Hashed Password (SHA-256): " + hashedPassword);
            System.out.println("---------------------------------");
        }
    }
}