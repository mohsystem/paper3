import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task46 {

    private static final int SALT_LENGTH_BYTES = 16;
    private static final int HASH_LENGTH_BITS = 256;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final String PBKDF2_ALGORITHM = "PBKDF2WithHmacSHA256";

    // In-memory storage for user data, using email as the unique key.
    private final Map<String, User> userDatabase = new HashMap<>();

    private static class User {
        final String name;
        final String email;
        final String passwordHash; // Stored as Base64
        final String salt; // Stored as Base64

        User(String name, String email, String passwordHash, String salt) {
            this.name = name;
            this.email = email;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }

    private static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        // RFC 5322 compliant regex
        String emailRegex = "^[a-zA-Z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-zA-Z0-9.-]+$";
        return Pattern.matches(emailRegex, email);
    }

    private static boolean isValidPassword(String password) {
        if (password == null || password.length() < 12) {
            return false;
        }
        // Requires at least one digit, one lowercase, one uppercase, one special character.
        String passwordRegex = "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{12,}$";
        return Pattern.matches(passwordRegex, password);
    }
    
    private static boolean isValidName(String name) {
        return name != null && !name.trim().isEmpty() && name.length() >= 2 && name.length() <= 50;
    }

    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        random.nextBytes(salt);
        return salt;
    }

    private static String hashPassword(String password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, HASH_LENGTH_BITS);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(PBKDF2_ALGORITHM);
        byte[] hash = factory.generateSecret(spec).getEncoded();
        return Base64.getEncoder().encodeToString(hash);
    }

    public boolean registerUser(String name, String email, String password) {
        if (!isValidName(name)) {
            System.err.println("Registration failed: Invalid name.");
            return false;
        }
        if (!isValidEmail(email)) {
            System.err.println("Registration failed: Invalid email format.");
            return false;
        }
        if (!isValidPassword(password)) {
            System.err.println("Registration failed: Password does not meet policy requirements (min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char).");
            return false;
        }

        if (userDatabase.containsKey(email)) {
            System.err.println("Registration failed: Email already exists.");
            return false;
        }

        try {
            byte[] saltBytes = generateSalt();
            String saltString = Base64.getEncoder().encodeToString(saltBytes);
            String passwordHash = hashPassword(password, saltBytes);

            User newUser = new User(name, email, passwordHash, saltString);
            userDatabase.put(email, newUser);

            System.out.println("User '" + name + "' registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Critical error during password hashing: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        Task46 system = new Task46();
        
        System.out.println("--- Test Case 1: Successful Registration ---");
        system.registerUser("Alice Smith", "alice.smith@example.com", "Password@12345");
        
        System.out.println("\n--- Test Case 2: Duplicate Email ---");
        system.registerUser("Alice Jones", "alice.smith@example.com", "AnotherP@ss123");
        
        System.out.println("\n--- Test Case 3: Weak Password (too short) ---");
        system.registerUser("Bob Johnson", "bob.j@example.com", "Weak@1");
        
        System.out.println("\n--- Test Case 4: Invalid Email ---");
        system.registerUser("Carol White", "carol.white@", "StrongPass!123");
        
        System.out.println("\n--- Test Case 5: Successful Registration 2 ---");
        system.registerUser("David Green", "david.green@example.com", "SecurePassword#2024");

        // Verify database state
        System.out.println("\n--- Verifying Database State ---");
        System.out.println("Is alice.smith@example.com registered? " + system.userDatabase.containsKey("alice.smith@example.com"));
        System.out.println("Is david.green@example.com registered? " + system.userDatabase.containsKey("david.green@example.com"));
        System.out.println("Is bob.j@example.com registered? " + system.userDatabase.containsKey("bob.j@example.com"));
    }
}