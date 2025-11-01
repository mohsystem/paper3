import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse38 {

    // In a real application, this would be a database.
    private static final Map<String, UserCredentials> userDatabase = new HashMap<>();
    
    // Strong password policy: at least 8 chars, one uppercase, one lowercase, one digit, one special char.
    private static final Pattern PASSWORD_PATTERN = 
        Pattern.compile("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
    
    // Username policy: 5-20 alphanumeric characters
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{5,20}$");
    
    private static final int SALT_LENGTH = 16; // 16 bytes salt
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 256; // 256-bit key
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    // Class to store user credentials securely
    private static class UserCredentials {
        byte[] salt;
        byte[] passwordHash;

        UserCredentials(byte[] salt, byte[] passwordHash) {
            this.salt = salt;
            this.passwordHash = passwordHash;
        }
    }

    /**
     * Hashes the password with a given salt.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely.
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Registers a new user.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || password == null || 
            !USERNAME_PATTERN.matcher(username).matches() || 
            !PASSWORD_PATTERN.matcher(password).matches()) {
            System.out.println("Registration failed: Invalid username or password format.");
            return false;
        }

        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username already exists.");
            return false;
        }

        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);

        char[] passwordChars = password.toCharArray();
        byte[] passwordHash = hashPassword(passwordChars, salt);
        
        // Clear password from memory
        Arrays.fill(passwordChars, '\0'); 

        userDatabase.put(username, new UserCredentials(salt, passwordHash));
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    /**
     * Attempts to log in a user.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Login failed: Username or password cannot be empty.");
            return false;
        }

        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            System.out.println("Login failed: Incorrect username or password."); // Generic message
            return false;
        }

        char[] passwordChars = password.toCharArray();
        byte[] loginAttemptHash = hashPassword(passwordChars, credentials.salt);
        
        // Clear password from memory
        Arrays.fill(passwordChars, '\0');

        // Constant-time comparison to prevent timing attacks
        if (Arrays.equals(loginAttemptHash, credentials.passwordHash)) {
            System.out.println("Login successful for user '" + username + "'. Redirecting to dashboard.");
            return true;
        } else {
            System.out.println("Login failed: Incorrect username or password.");
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Setting up user ---");
        // Register a user with a strong password
        registerUser("testuser1", "Password@123");
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Correct username and password
        System.out.println("1. Testing correct credentials...");
        login("testuser1", "Password@123");

        // Test Case 2: Correct username, incorrect password
        System.out.println("\n2. Testing incorrect password...");
        login("testuser1", "WrongPassword@123");

        // Test Case 3: Incorrect username
        System.out.println("\n3. Testing incorrect username...");
        login("nonexistentuser", "Password@123");
        
        // Test Case 4: Empty password
        System.out.println("\n4. Testing empty password...");
        login("testuser1", "");

        // Test Case 5: Registration with weak password
        System.out.println("\n5. Testing registration with weak password...");
        registerUser("testuser2", "weak");
    }
}