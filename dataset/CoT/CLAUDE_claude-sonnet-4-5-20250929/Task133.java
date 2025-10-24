
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.time.Instant;
import java.time.temporal.ChronoUnit;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task133 {
    private static final int TOKEN_EXPIRY_MINUTES = 15;
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final Pattern PASSWORD_PATTERN = Pattern.compile(
        "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$"
    );
    
    private Map<String, UserAccount> userDatabase;
    private Map<String, ResetToken> tokenDatabase;
    private SecureRandom secureRandom;
    
    static class UserAccount {
        String email;
        String passwordHash;
        String salt;
        
        UserAccount(String email, String passwordHash, String salt) {
            this.email = email;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    static class ResetToken {
        String tokenHash;
        String email;
        Instant expiryTime;
        boolean used;
        
        ResetToken(String tokenHash, String email, Instant expiryTime) {
            this.tokenHash = tokenHash;
            this.email = email;
            this.expiryTime = expiryTime;
            this.used = false;
        }
    }
    
    public Task133() {
        this.userDatabase = new HashMap<>();
        this.tokenDatabase = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    private String generateSalt() {
        byte[] salt = new byte[16];
        secureRandom.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private String hashPassword(String password, String salt) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.update(salt.getBytes());
            byte[] hash = digest.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Hashing algorithm not available", e);
        }
    }
    
    private String generateSecureToken() {
        byte[] token = new byte[32];
        secureRandom.nextBytes(token);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(token);
    }
    
    public boolean registerUser(String email, String password) {
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        
        if (!isPasswordValid(password)) {
            return false;
        }
        
        if (userDatabase.containsKey(email.toLowerCase())) {
            return false;
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        userDatabase.put(email.toLowerCase(), new UserAccount(email.toLowerCase(), passwordHash, salt));
        return true;
    }
    
    public String requestPasswordReset(String email) {
        if (email == null || email.trim().isEmpty()) {
            return null;
        }
        
        String normalizedEmail = email.toLowerCase();
        
        if (!userDatabase.containsKey(normalizedEmail)) {
            return null;
        }
        
        String token = generateSecureToken();
        String tokenHash = hashPassword(token, "");
        Instant expiryTime = Instant.now().plus(TOKEN_EXPIRY_MINUTES, ChronoUnit.MINUTES);
        
        tokenDatabase.put(token, new ResetToken(tokenHash, normalizedEmail, expiryTime));
        
        return token;
    }
    
    public boolean resetPassword(String token, String newPassword) {
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        
        if (!isPasswordValid(newPassword)) {
            return false;
        }
        
        ResetToken resetToken = tokenDatabase.get(token);
        
        if (resetToken == null) {
            return false;
        }
        
        if (resetToken.used) {
            return false;
        }
        
        if (Instant.now().isAfter(resetToken.expiryTime)) {
            tokenDatabase.remove(token);
            return false;
        }
        
        UserAccount user = userDatabase.get(resetToken.email);
        if (user == null) {
            return false;
        }
        
        String newSalt = generateSalt();
        String newPasswordHash = hashPassword(newPassword, newSalt);
        user.passwordHash = newPasswordHash;
        user.salt = newSalt;
        
        resetToken.used = true;
        tokenDatabase.remove(token);
        
        return true;
    }
    
    public boolean isPasswordValid(String password) {
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        
        return PASSWORD_PATTERN.matcher(password).matches();
    }
    
    public boolean verifyPassword(String email, String password) {
        if (email == null || password == null) {
            return false;
        }
        
        UserAccount user = userDatabase.get(email.toLowerCase());
        if (user == null) {
            return false;
        }
        
        String hashedPassword = hashPassword(password, user.salt);
        return hashedPassword.equals(user.passwordHash);
    }
    
    public static void main(String[] args) {
        Task133 passwordReset = new Task133();
        
        System.out.println("=== Test Case 1: Register users ===");
        boolean result1 = passwordReset.registerUser("user1@example.com", "SecurePass123!");
        System.out.println("Register user1: " + result1);
        boolean result2 = passwordReset.registerUser("user2@example.com", "AnotherPass456@");
        System.out.println("Register user2: " + result2);
        
        System.out.println("\\n=== Test Case 2: Request password reset ===");
        String token1 = passwordReset.requestPasswordReset("user1@example.com");
        System.out.println("Reset token generated: " + (token1 != null));
        
        System.out.println("\\n=== Test Case 3: Reset password with valid token ===");
        boolean reset1 = passwordReset.resetPassword(token1, "NewSecurePass789!");
        System.out.println("Password reset successful: " + reset1);
        boolean verify1 = passwordReset.verifyPassword("user1@example.com", "NewSecurePass789!");
        System.out.println("New password verified: " + verify1);
        
        System.out.println("\\n=== Test Case 4: Try to reuse token ===");
        boolean reset2 = passwordReset.resetPassword(token1, "AnotherNewPass000!");
        System.out.println("Reuse token (should fail): " + reset2);
        
        System.out.println("\\n=== Test Case 5: Invalid password reset ===");
        String token2 = passwordReset.requestPasswordReset("user2@example.com");
        boolean reset3 = passwordReset.resetPassword(token2, "weak");
        System.out.println("Reset with weak password (should fail): " + reset3);
        boolean reset4 = passwordReset.resetPassword("invalid_token", "ValidPass123!");
        System.out.println("Reset with invalid token (should fail): " + reset4);
    }
}
