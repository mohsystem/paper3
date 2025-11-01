import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import java.util.Arrays;

public class Lse97 {

    private static final Map<String, User> userDatabase = new ConcurrentHashMap<>();
    private static final int HASH_ITERATIONS = 210000;
    private static final int HASH_KEY_LENGTH = 256;
    private static final int SALT_SIZE = 16;

    // Strong password policy: min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char.
    private static final Pattern PASSWORD_PATTERN = Pattern.compile(
            "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{12,}$");

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{3,20}$");

    // A simple email regex for demonstration purposes.
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
            "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");

    private static class User {
        final String username;
        final String salt; // Stored as Base64 string
        final String hashedPassword; // Stored as Base64 string
        final String firstName;
        final String lastName;
        final String email;

        User(String username, String salt, String hashedPassword, String firstName, String lastName, String email) {
            this.username = username;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
        }
    }

    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        random.nextBytes(salt);
        return salt;
    }

    private static String hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, HASH_ITERATIONS, HASH_KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // This should not happen with standard algorithms
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            return "Registration failed: Invalid username format. Must be 3-20 alphanumeric characters.";
        }
        if (password == null || !PASSWORD_PATTERN.matcher(password).matches()) {
            return "Registration failed: Password does not meet policy. Must be at least 12 characters and contain an uppercase letter, a lowercase letter, a digit, and a special character.";
        }
        if (firstName == null || firstName.trim().isEmpty() || firstName.length() > 50) {
            return "Registration failed: Invalid first name.";
        }
        if (lastName == null || lastName.trim().isEmpty() || lastName.length() > 50) {
            return "Registration failed: Invalid last name.";
        }
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            return "Registration failed: Invalid email format.";
        }

        if (userDatabase.containsKey(username)) {
            return "Registration failed: Username already exists.";
        }

        char[] passwordChars = password.toCharArray();
        try {
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(passwordChars, salt);
            String saltString = Base64.getEncoder().encodeToString(salt);

            User newUser = new User(username, saltString, hashedPassword, firstName, lastName, email);
            userDatabase.put(username, newUser);

            return "Registration succeeded for username: " + username;
        } finally {
            // Securely clear the password from memory
            Arrays.fill(passwordChars, '\0');
        }
    }
    
    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful Registration
        System.out.println("Test 1: " + registerUser("john_doe", "Password@1234", "John", "Doe", "john.doe@example.com"));

        // Test Case 2: Username already exists
        System.out.println("Test 2: " + registerUser("john_doe", "AnotherPass@5678", "John", "Doe", "john.d@example.com"));

        // Test Case 3: Weak password
        System.out.println("Test 3: " + registerUser("jane_doe", "weak", "Jane", "Doe", "jane.doe@example.com"));

        // Test Case 4: Invalid username
        System.out.println("Test 4: " + registerUser("jd", "StrongPass@123", "Jane", "Doe", "jane.doe@example.com"));
        
        // Test Case 5: Invalid email
        System.out.println("Test 5: " + registerUser("peter_jones", "StrongPass@123", "Peter", "Jones", "peterjones@invalid"));

        System.out.println("\n--- Database Content (for verification) ---");
        userDatabase.forEach((key, value) -> 
            System.out.println("Username: " + value.username + ", Hashed Password: " + value.hashedPassword.substring(0, 10) + "...")
        );
    }
}