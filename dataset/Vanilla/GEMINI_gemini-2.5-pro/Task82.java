import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Formatter;

public class Task82 {

    /**
     * Computes the SHA-256 hash of a password combined with a salt.
     *
     * @param password The password to hash.
     * @param salt The salt to combine with the password.
     * @return The hexadecimal representation of the hash, or null on error.
     */
    public static String hashPassword(String password, String salt) {
        try {
            String combined = salt + password;
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(combined.getBytes(StandardCharsets.UTF_8));
            
            // Convert byte array to hex string
            StringBuilder hexString = new StringBuilder(2 * hash.length);
            for (byte b : hash) {
                hexString.append(String.format("%02x", b));
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        // Test Case 1
        String pass1 = "password123";
        String salt1 = "randomsalt1";
        System.out.println("Password: " + pass1 + ", Salt: " + salt1);
        System.out.println("Hash: " + hashPassword(pass1, salt1));
        System.out.println();

        // Test Case 2
        String pass2 = "MyP@ssw0rd";
        String salt2 = "salty-salty";
        System.out.println("Password: " + pass2 + ", Salt: " + salt2);
        System.out.println("Hash: " + hashPassword(pass2, salt2));
        System.out.println();

        // Test Case 3
        String pass3 = "topsecret";
        String salt3 = "another-salt";
        System.out.println("Password: " + pass3 + ", Salt: " + salt3);
        System.out.println("Hash: " + hashPassword(pass3, salt3));
        System.out.println();

        // Test Case 4 (Empty Password)
        String pass4 = "";
        String salt4 = "emptypass";
        System.out.println("Password: " + pass4 + ", Salt: " + salt4);
        System.out.println("Hash: " + hashPassword(pass4, salt4));
        System.out.println();

        // Test Case 5 (Empty Salt)
        String pass5 = "short";
        String salt5 = "";
        System.out.println("Password: " + pass5 + ", Salt: " + salt5);
        System.out.println("Hash: " + hashPassword(pass5, salt5));
        System.out.println();
    }
}