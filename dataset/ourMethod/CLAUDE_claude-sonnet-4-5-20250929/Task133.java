
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task133 {
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int HASH_LENGTH = 256;
    private static final int TOKEN_LENGTH = 32;
    private static final long TOKEN_EXPIRY_MS = 3600000; // 1 hour
    
    private static final Pattern PASSWORD_PATTERN = Pattern.compile(
        "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{12,}$"
    );
    
    private static class User {
        String username;
        byte[] passwordHash;
        byte[] salt;
        
        User(String username, byte[] passwordHash, byte[] salt) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    private static class ResetToken {
        String username;
        byte[] tokenHash;
        long expiryTime;
        
        ResetToken(String username, byte[] tokenHash, long expiryTime) {
            this.username = username;
            this.tokenHash = tokenHash;
            this.expiryTime = expiryTime;
        }
    }
    
    private Map<String, User> users;
    private Map<String, ResetToken> resetTokens;
    private SecureRandom secureRandom;
    
    public Task133() {
        this.users = new HashMap<>();
        this.resetTokens = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    private byte[] generateSalt() {
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        return salt;
    }
    
    private byte[] hashPassword(String password, byte[] salt) 
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(
            password.toCharArray(), 
            salt, 
            PBKDF2_ITERATIONS, 
            HASH_LENGTH
        );
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return hash;
        } finally {
            spec.clearPassword();
        }
    }
    
    private boolean validatePassword(String password) {
        if (password == null || password.length() < 12 || password.length() > 128) {
            return false;
        }
        return PASSWORD_PATTERN.matcher(password).matches();
    }
    
    private boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    public String registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || username.length() > 50) {
            return "Error: Invalid username";
        }
        
        username = username.trim();
        
        if (!validatePassword(password)) {
            return "Error: Password must be 12-128 characters with uppercase, lowercase, digit, and special character";
        }
        
        if (users.containsKey(username)) {
            return "Error: User already exists";
        }
        
        try {
            byte[] salt = generateSalt();
            byte[] hash = hashPassword(password, salt);
            users.put(username, new User(username, hash, salt));
            return "Success: User registered";
        } catch (Exception e) {
            return "Error: Registration failed";
        }
    }
    
    public String requestPasswordReset(String username) {
        if (username == null || username.trim().isEmpty()) {
            return "Error: Invalid username";
        }
        
        username = username.trim();
        
        if (!users.containsKey(username)) {
            return "Error: User not found";
        }
        
        try {
            byte[] tokenBytes = new byte[TOKEN_LENGTH];
            secureRandom.nextBytes(tokenBytes);
            String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
            
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] tokenHash = digest.digest(token.getBytes(StandardCharsets.UTF_8));
            
            long expiryTime = System.currentTimeMillis() + TOKEN_EXPIRY_MS;
            resetTokens.put(username, new ResetToken(username, tokenHash, expiryTime));
            
            return "Success: Reset token generated: " + token;
        } catch (Exception e) {
            return "Error: Token generation failed";
        }
    }
    
    public String resetPassword(String username, String token, String newPassword) {
        if (username == null || username.trim().isEmpty()) {
            return "Error: Invalid username";
        }
        
        if (token == null || token.trim().isEmpty()) {
            return "Error: Invalid token";
        }
        
        username = username.trim();
        token = token.trim();
        
        if (!validatePassword(newPassword)) {
            return "Error: Password must be 12-128 characters with uppercase, lowercase, digit, and special character";
        }
        
        if (!users.containsKey(username)) {
            return "Error: User not found";
        }
        
        if (!resetTokens.containsKey(username)) {
            return "Error: No reset token found";
        }
        
        ResetToken resetToken = resetTokens.get(username);
        
        if (System.currentTimeMillis() > resetToken.expiryTime) {
            resetTokens.remove(username);
            return "Error: Token expired";
        }
        
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] providedTokenHash = digest.digest(token.getBytes(StandardCharsets.UTF_8));
            
            if (!constantTimeEquals(providedTokenHash, resetToken.tokenHash)) {
                return "Error: Invalid token";
            }
            
            byte[] newSalt = generateSalt();
            byte[] newHash = hashPassword(newPassword, newSalt);
            
            User user = users.get(username);
            user.passwordHash = newHash;
            user.salt = newSalt;
            
            resetTokens.remove(username);
            
            return "Success: Password reset successfully";
        } catch (Exception e) {
            return "Error: Password reset failed";
        }
    }
    
    public String verifyLogin(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null) {
            return "Error: Invalid credentials";
        }
        
        username = username.trim();
        
        if (!users.containsKey(username)) {
            return "Error: Invalid credentials";
        }
        
        try {
            User user = users.get(username);
            byte[] hash = hashPassword(password, user.salt);
            
            if (constantTimeEquals(hash, user.passwordHash)) {
                return "Success: Login successful";
            } else {
                return "Error: Invalid credentials";
            }
        } catch (Exception e) {
            return "Error: Login failed";
        }
    }
    
    public static void main(String[] args) {
        Task133 system = new Task133();
        
        System.out.println("Test 1: Register user");
        System.out.println(system.registerUser("alice", "SecurePass123!@#"));
        
        System.out.println("\\nTest 2: Request password reset");
        String resetResponse = system.requestPasswordReset("alice");
        System.out.println(resetResponse);
        
        if (resetResponse.startsWith("Success")) {
            String token = resetResponse.substring(resetResponse.lastIndexOf(": ") + 2);
            
            System.out.println("\\nTest 3: Reset password with valid token");
            System.out.println(system.resetPassword("alice", token, "NewSecurePass456!@#"));
            
            System.out.println("\\nTest 4: Verify login with new password");
            System.out.println(system.verifyLogin("alice", "NewSecurePass456!@#"));
        }
        
        System.out.println("\\nTest 5: Attempt reset with invalid token");
        System.out.println(system.resetPassword("alice", "invalidtoken123", "AnotherPass789!@#"));
    }
}
