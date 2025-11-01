import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;

class User {
    private String username;
    private String email;
    private String passwordHash;
    private byte[] salt;

    public User(String username, String email, String password) {
        if (username == null || username.trim().isEmpty() || 
            email == null || email.trim().isEmpty() || 
            password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Username, email, and password cannot be empty.");
        }
        this.username = username;
        this.email = email;
        this.salt = generateSalt();
        this.passwordHash = hashPassword(password, this.salt);
    }

    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    private String hashPassword(String password, byte[] salt) {
        try {
            // Use PBKDF2 for password hashing
            PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 65536, 256);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = skf.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely.
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    public boolean verifyPassword(String password) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        String newHash = hashPassword(password, this.salt);
        // This is not a constant-time comparison but is acceptable for password hashes
        // of a fixed length. For higher security, one might decode to bytes and use
        // MessageDigest.isEqual().
        return this.passwordHash.equals(newHash);
    }
    
    public String getUsername() { return username; }
    public String getEmail() { return email; }
    public void setEmail(String email) { this.email = email; }
}

class LoginSystem {
    private User currentUser;
    private boolean isLoggedIn = false;

    public boolean register(String username, String email, String password) {
        try {
            // For this example, registering overwrites the current user.
            // A real system would check for existing users in a database.
            this.currentUser = new User(username, email, password);
            this.isLoggedIn = false; // Registration does not automatically log in
            System.out.println("User " + username + " registered successfully.");
            return true;
        } catch (IllegalArgumentException e) {
            System.out.println("Registration failed: " + e.getMessage());
            return false;
        }
    }

    public boolean login(String username, String password) {
        if (currentUser == null || !currentUser.getUsername().equals(username)) {
            System.out.println("Login failed: User not found.");
            isLoggedIn = false;
            return false;
        }
        if (currentUser.verifyPassword(password)) {
            isLoggedIn = true;
            System.out.println("Login successful for user " + username + ".");
            return true;
        } else {
            isLoggedIn = false;
            System.out.println("Login failed: Incorrect password.");
            return false;
        }
    }

    public void logout() {
        isLoggedIn = false;
        System.out.println("User logged out.");
    }

    public boolean changeEmail(String oldEmail, String password, String newEmail) {
        if (!isLoggedIn) {
            System.out.println("Email change failed: User is not logged in.");
            return false;
        }
        if (newEmail == null || newEmail.trim().isEmpty()) {
             System.out.println("Email change failed: New email cannot be empty.");
             return false;
        }
        // Re-authenticate user before performing sensitive action
        if (!currentUser.getEmail().equals(oldEmail)) {
            System.out.println("Email change failed: Old email does not match.");
            return false;
        }
        if (!currentUser.verifyPassword(password)) {
            System.out.println("Email change failed: Incorrect password.");
            return false;
        }

        currentUser.setEmail(newEmail);
        System.out.println("Email for " + currentUser.getUsername() + " changed successfully to " + newEmail + ".");
        return true;
    }
    
    public User getCurrentUser() {
        return isLoggedIn ? currentUser : null;
    }
}

public class Lse58 {
    public static void main(String[] args) {
        LoginSystem system = new LoginSystem();

        // Setup a user. In a real application, passwords would not be hardcoded.
        system.register("testuser", "initial@example.com", "Password123!");

        System.out.println("\n--- Test Case 1: Successful Email Change ---");
        system.login("testuser", "Password123!");
        if (system.getCurrentUser() != null) {
            System.out.println("Current email: " + system.getCurrentUser().getEmail());
            system.changeEmail("initial@example.com", "Password123!", "new@example.com");
            System.out.println("New email: " + system.getCurrentUser().getEmail());
        }
        system.logout();

        System.out.println("\n--- Test Case 2: Change Email with Wrong Password ---");
        system.login("testuser", "Password123!");
        if (system.getCurrentUser() != null) {
            System.out.println("Current email: " + system.getCurrentUser().getEmail());
            system.changeEmail("new@example.com", "WrongPassword!", "another@example.com");
            System.out.println("Email after failed attempt: " + system.getCurrentUser().getEmail());
        }
        system.logout();

        System.out.println("\n--- Test Case 3: Change Email with Wrong Old Email ---");
        system.login("testuser", "Password123!");
        if (system.getCurrentUser() != null) {
            System.out.println("Current email: " + system.getCurrentUser().getEmail());
            system.changeEmail("wrong.old@example.com", "Password123!", "another@example.com");
            System.out.println("Email after failed attempt: " + system.getCurrentUser().getEmail());
        }
        system.logout();
        
        System.out.println("\n--- Test Case 4: Change Email When Not Logged In ---");
        // Ensure user is logged out before attempt
        system.changeEmail("new@example.com", "Password123!", "another@example.com");
        
        // Log in to check if email was changed (it shouldn't have been)
        system.login("testuser", "Password123!");
        if (system.getCurrentUser() != null) {
             System.out.println("Email after failed attempt: " + system.getCurrentUser().getEmail());
        }
        system.logout();
        
        System.out.println("\n--- Test Case 5: Login with Wrong Password ---");
        system.login("testuser", "NotThePassword");
        // Attempt to change email (should fail because user is not logged in)
        system.changeEmail("new@example.com", "Password123!", "another@example.com");
    }
}