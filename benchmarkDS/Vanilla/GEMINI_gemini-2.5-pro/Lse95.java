import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse95 {

    private static final int SALT_LENGTH = 16;
    private final Map<String, UserData> database = new HashMap<>();

    private static class UserData {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public UserData(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        public byte[] getSalt() {
            return salt;
        }

        public byte[] getHashedPassword() {
            return hashedPassword;
        }
    }

    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] passwordBytes = password.getBytes(StandardCharsets.UTF_8);
            
            // Append salt to password bytes before hashing
            byte[] combined = new byte[passwordBytes.length + salt.length];
            System.arraycopy(passwordBytes, 0, combined, 0, passwordBytes.length);
            System.arraycopy(salt, 0, combined, passwordBytes.length, salt.length);

            return md.digest(combined);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    public boolean registerUser(String username, String password) {
        if (database.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        database.put(username, new UserData(salt, hashedPassword));
        System.out.println("Registration successful for username: " + username);
        return true;
    }

    public boolean verifyLogin(String username, String password) {
        UserData userData = database.get(username);
        if (userData == null) {
            System.out.println("Login failed: Username '" + username + "' not found.");
            return false;
        }
        byte[] salt = userData.getSalt();
        byte[] storedHashedPassword = userData.getHashedPassword();
        byte[] enteredHashedPassword = hashPassword(password, salt);

        if (Arrays.equals(storedHashedPassword, enteredHashedPassword)) {
            System.out.println("Login successful for username: " + username);
            return true;
        } else {
            System.out.println("Login failed: Incorrect password for username '" + username + "'.");
            return false;
        }
    }

    public static void main(String[] args) {
        Lse95 authSystem = new Lse95();
        
        System.out.println("--- Test Case 1: Successful Registration ---");
        authSystem.registerUser("alice", "password123");
        
        System.out.println("\n--- Test Case 2: Successful Login ---");
        authSystem.verifyLogin("alice", "password123");
        
        System.out.println("\n--- Test Case 3: Failed Login (Incorrect Password) ---");
        authSystem.verifyLogin("alice", "wrongpassword");

        System.out.println("\n--- Test Case 4: Failed Login (User Not Found) ---");
        authSystem.verifyLogin("bob", "password123");

        System.out.println("\n--- Test Case 5: Failed Registration (Username Exists) ---");
        authSystem.registerUser("alice", "anotherpassword");
    }
}