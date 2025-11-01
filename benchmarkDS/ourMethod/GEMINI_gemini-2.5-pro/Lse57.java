import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

class User {
    private String username;
    private String email;
    private byte[] passwordHash;
    private byte[] salt;

    public User(String username, String email, byte[] passwordHash, byte[] salt) {
        this.username = username;
        this.email = email;
        this.passwordHash = passwordHash;
        this.salt = salt;
    }

    // Getters
    public String getUsername() { return username; }
    public String getEmail() { return email; }
    public byte[] getPasswordHash() { return passwordHash; }
    public byte[] getSalt() { return salt; }

    // Setter for email
    public void setEmail(String email) {
        this.email = email;
    }
}

class UserService {
    private final Map<String, User> users = new HashMap<>();
    private User loggedInUser = null;

    private static final int SALT_LENGTH = 16;
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 128;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
    
    // Simple email regex for validation
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");

    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATION_COUNT, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
        return factory.generateSecret(spec).getEncoded();
    }

    private boolean verifyPassword(String password, byte[] originalHash, byte[] salt) {
        try {
            byte[] comparisonHash = hashPassword(password, salt);
            return Arrays.equals(originalHash, comparisonHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // Log the exception in a real application
            System.err.println("Error during password verification: " + e.getMessage());
            return false;
        }
    }

    public boolean registerUser(String username, String password, String email) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty() ||
            email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            System.out.println("Registration failed: Invalid input.");
            return false;
        }
        if (users.containsKey(username)) {
            System.out.println("Registration failed: Username already exists.");
            return false;
        }
        try {
            byte[] salt = generateSalt();
            byte[] hash = hashPassword(password, salt);
            User newUser = new User(username, email, hash, salt);
            users.put(username, newUser);
            System.out.println("User " + username + " registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during user registration: " + e.getMessage());
            return false;
        }
    }

    public boolean login(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return false;
        }
        User user = users.get(username);
        if (user != null && verifyPassword(password, user.getPasswordHash(), user.getSalt())) {
            loggedInUser = user;
            System.out.println(username + " logged in successfully.");
            return true;
        }
        System.out.println("Login failed for " + username + ".");
        return false;
    }
    
    public void logout() {
        if (loggedInUser != null) {
            System.out.println(loggedInUser.getUsername() + " logged out.");
            loggedInUser = null;
        }
    }

    public boolean changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // Rule #1: Input Validation
        if (oldEmail == null || oldEmail.trim().isEmpty() || 
            newEmail == null || !EMAIL_PATTERN.matcher(newEmail).matches() ||
            confirmPassword == null || confirmPassword.isEmpty()) {
            System.out.println("Email change failed: Invalid input provided.");
            return false;
        }

        if (loggedInUser == null) {
            System.out.println("Email change failed: User must be logged in.");
            return false;
        }
        
        if (!loggedInUser.getEmail().equals(oldEmail)) {
            System.out.println("Email change failed: Old email does not match.");
            return false;
        }

        if (!verifyPassword(confirmPassword, loggedInUser.getPasswordHash(), loggedInUser.getSalt())) {
            System.out.println("Email change failed: Incorrect password.");
            return false;
        }

        loggedInUser.setEmail(newEmail);
        System.out.println("Email for user " + loggedInUser.getUsername() + " changed successfully to " + newEmail);
        return true;
    }
    
    public void printUserDetails(String username) {
        User user = users.get(username);
        if (user != null) {
            System.out.println("Details for " + username + ": Email = " + user.getEmail());
        } else {
            System.out.println("User " + username + " not found.");
        }
    }
}


public class Lse57 {
    public static void main(String[] args) {
        UserService service = new UserService();
        
        // Setup: Register a user
        service.registerUser("testuser", "Password123!", "initial@example.com");
        System.out.println("----------------------------------------");
        
        // Test Case 1: Successful email change
        System.out.println("Test Case 1: Successful email change");
        service.login("testuser", "Password123!");
        service.printUserDetails("testuser");
        service.changeEmail("initial@example.com", "updated@example.com", "Password123!");
        service.printUserDetails("testuser");
        service.logout();
        System.out.println("----------------------------------------");

        // Test Case 2: Attempt to change email when not logged in
        System.out.println("Test Case 2: Attempt change when not logged in");
        service.changeEmail("updated@example.com", "another@example.com", "Password123!");
        service.printUserDetails("testuser");
        System.out.println("----------------------------------------");

        // Test Case 3: Attempt to change email with wrong password
        System.out.println("Test Case 3: Attempt change with wrong password");
        service.login("testuser", "Password123!");
        service.changeEmail("updated@example.com", "another@example.com", "WrongPassword!");
        service.printUserDetails("testuser");
        service.logout();
        System.out.println("----------------------------------------");

        // Test Case 4: Attempt to change email with wrong old email
        System.out.println("Test Case 4: Attempt change with wrong old email");
        service.login("testuser", "Password123!");
        service.changeEmail("wrong.old@example.com", "another@example.com", "Password123!");
        service.printUserDetails("testuser");
        service.logout();
        System.out.println("----------------------------------------");
        
        // Test Case 5: Attempt to change to an invalid new email
        System.out.println("Test Case 5: Attempt change to an invalid new email");
        service.login("testuser", "Password123!");
        service.changeEmail("updated@example.com", "invalid-email", "Password123!");
        service.printUserDetails("testuser");
        service.logout();
        System.out.println("----------------------------------------");
    }
}