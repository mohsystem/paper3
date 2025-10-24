
import java.security.SecureRandom;
import java.time.LocalDateTime;
import java.time.temporal.ChronoUnit;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task133 {
    private static final int TOKEN_LENGTH = 32;
    private static final int TOKEN_EXPIRY_MINUTES = 15;
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );
    private static final Pattern PASSWORD_PATTERN = Pattern.compile(
        "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$"
    );
    
    private static Map<String, User> users = new HashMap<>();
    private static Map<String, ResetToken> resetTokens = new HashMap<>();
    
    static class User {
        String email;
        String passwordHash;
        
        User(String email, String passwordHash) {
            this.email = email;
            this.passwordHash = passwordHash;
        }
    }
    
    static class ResetToken {
        String token;
        String email;
        LocalDateTime expiryTime;
        
        ResetToken(String token, String email, LocalDateTime expiryTime) {
            this.token = token;
            this.email = email;
            this.expiryTime = expiryTime;
        }
        
        boolean isExpired() {
            return LocalDateTime.now().isAfter(expiryTime);
        }
    }
    
    public static String generateResetToken(String email) {
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email format";
        }
        
        if (!users.containsKey(email)) {
            return "Error: Email not found";
        }
        
        SecureRandom random = new SecureRandom();
        StringBuilder token = new StringBuilder();
        String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            token.append(chars.charAt(random.nextInt(chars.length())));
        }
        
        String tokenStr = token.toString();
        LocalDateTime expiryTime = LocalDateTime.now().plus(TOKEN_EXPIRY_MINUTES, ChronoUnit.MINUTES);
        resetTokens.put(tokenStr, new ResetToken(tokenStr, email, expiryTime));
        
        return tokenStr;
    }
    
    public static String resetPassword(String token, String newPassword) {
        if (token == null || token.isEmpty()) {
            return "Error: Token is required";
        }
        
        if (newPassword == null || !PASSWORD_PATTERN.matcher(newPassword).matches()) {
            return "Error: Password must be at least 8 characters with uppercase, lowercase, digit, and special character";
        }
        
        ResetToken resetToken = resetTokens.get(token);
        
        if (resetToken == null) {
            return "Error: Invalid token";
        }
        
        if (resetToken.isExpired()) {
            resetTokens.remove(token);
            return "Error: Token has expired";
        }
        
        User user = users.get(resetToken.email);
        user.passwordHash = hashPassword(newPassword);
        resetTokens.remove(token);
        
        return "Success: Password reset successfully";
    }
    
    private static String hashPassword(String password) {
        return "HASHED_" + password;
    }
    
    public static String registerUser(String email, String password) {
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email format";
        }
        
        if (users.containsKey(email)) {
            return "Error: User already exists";
        }
        
        users.put(email, new User(email, hashPassword(password)));
        return "Success: User registered";
    }
    
    public static void main(String[] args) {
        System.out.println("=== Password Reset Functionality Tests ===\\n");
        
        // Test Case 1: Register users
        System.out.println("Test 1: Register users");
        System.out.println(registerUser("user1@example.com", "Password123!"));
        System.out.println(registerUser("user2@example.com", "Secure456@"));
        System.out.println();
        
        // Test Case 2: Generate reset token for valid user
        System.out.println("Test 2: Generate reset token for valid user");
        String token1 = generateResetToken("user1@example.com");
        System.out.println("Token generated: " + (token1.startsWith("Error") ? token1 : "Success (Length: " + token1.length() + ")"));
        System.out.println();
        
        // Test Case 3: Generate reset token for invalid email
        System.out.println("Test 3: Generate reset token for invalid email");
        System.out.println(generateResetToken("invalidemail"));
        System.out.println();
        
        // Test Case 4: Reset password with valid token and valid password
        System.out.println("Test 4: Reset password with valid token");
        String token2 = generateResetToken("user2@example.com");
        if (!token2.startsWith("Error")) {
            System.out.println(resetPassword(token2, "NewPass789#"));
        }
        System.out.println();
        
        // Test Case 5: Reset password with invalid token
        System.out.println("Test 5: Reset password with invalid token");
        System.out.println(resetPassword("invalidtoken123", "ValidPass123!"));
        System.out.println();
        
        // Test Case 6: Reset password with weak password
        System.out.println("Test 6: Reset password with weak password");
        String token3 = generateResetToken("user1@example.com");
        if (!token3.startsWith("Error")) {
            System.out.println(resetPassword(token3, "weak"));
        }
    }
}
