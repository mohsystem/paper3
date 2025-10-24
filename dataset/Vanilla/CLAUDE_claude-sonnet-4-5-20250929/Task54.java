
import java.util.*;
import java.security.SecureRandom;
import java.time.LocalDateTime;
import java.time.temporal.ChronoUnit;

class Task54 {
    private static Map<String, User> users = new HashMap<>();
    private static Map<String, OTP> otpStore = new HashMap<>();
    private static final int OTP_EXPIRY_MINUTES = 5;
    private static final int OTP_LENGTH = 6;
    
    static class User {
        String username;
        String password;
        String email;
        
        User(String username, String password, String email) {
            this.username = username;
            this.password = password;
            this.email = email;
        }
    }
    
    static class OTP {
        String code;
        LocalDateTime generatedAt;
        
        OTP(String code, LocalDateTime generatedAt) {
            this.code = code;
            this.generatedAt = generatedAt;
        }
        
        boolean isExpired() {
            return ChronoUnit.MINUTES.between(generatedAt, LocalDateTime.now()) >= OTP_EXPIRY_MINUTES;
        }
    }
    
    public static String generateOTP() {
        SecureRandom random = new SecureRandom();
        StringBuilder otp = new StringBuilder();
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp.append(random.nextInt(10));
        }
        return otp.toString();
    }
    
    public static boolean registerUser(String username, String password, String email) {
        if (users.containsKey(username)) {
            return false;
        }
        users.put(username, new User(username, password, email));
        return true;
    }
    
    public static String authenticateCredentials(String username, String password) {
        User user = users.get(username);
        if (user == null || !user.password.equals(password)) {
            return null;
        }
        
        String otp = generateOTP();
        otpStore.put(username, new OTP(otp, LocalDateTime.now()));
        return otp;
    }
    
    public static boolean verifyOTP(String username, String enteredOTP) {
        OTP otp = otpStore.get(username);
        if (otp == null) {
            return false;
        }
        
        if (otp.isExpired()) {
            otpStore.remove(username);
            return false;
        }
        
        if (otp.code.equals(enteredOTP)) {
            otpStore.remove(username);
            return true;
        }
        
        return false;
    }
    
    public static String performLogin(String username, String password, String enteredOTP) {
        String generatedOTP = authenticateCredentials(username, password);
        if (generatedOTP == null) {
            return "Login Failed: Invalid credentials";
        }
        
        if (verifyOTP(username, enteredOTP)) {
            return "Login Successful";
        } else {
            return "Login Failed: Invalid or expired OTP";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Two-Factor Authentication System ===\\n");
        
        // Test Case 1: Successful registration and login
        System.out.println("Test Case 1: Successful registration and login");
        registerUser("user1", "pass123", "user1@example.com");
        String otp1 = authenticateCredentials("user1", "pass123");
        System.out.println("Generated OTP: " + otp1);
        String result1 = performLogin("user1", "pass123", otp1);
        System.out.println("Result: " + result1 + "\\n");
        
        // Test Case 2: Wrong password
        System.out.println("Test Case 2: Wrong password");
        registerUser("user2", "pass456", "user2@example.com");
        String result2 = performLogin("user2", "wrongpass", "123456");
        System.out.println("Result: " + result2 + "\\n");
        
        // Test Case 3: Wrong OTP
        System.out.println("Test Case 3: Wrong OTP");
        registerUser("user3", "pass789", "user3@example.com");
        String otp3 = authenticateCredentials("user3", "pass789");
        System.out.println("Generated OTP: " + otp3);
        String result3 = performLogin("user3", "pass789", "000000");
        System.out.println("Result: " + result3 + "\\n");
        
        // Test Case 4: User doesn't exist\n        System.out.println("Test Case 4: User doesn't exist");\n        String result4 = performLogin("nonexistent", "pass000", "123456");\n        System.out.println("Result: " + result4 + "\
");\n        \n        // Test Case 5: Multiple users with successful login\n        System.out.println("Test Case 5: Multiple users with successful login");\n        registerUser("user4", "secure123", "user4@example.com");\n        registerUser("user5", "secure456", "user5@example.com");\n        String otp4 = authenticateCredentials("user4", "secure123");\n        String otp5 = authenticateCredentials("user5", "secure456");\n        System.out.println("User4 OTP: " + otp4);\n        System.out.println("User5 OTP: " + otp5);\n        String result5a = performLogin("user4", "secure123", otp4);\n        String result5b = performLogin("user5", "secure456", otp5);\n        System.out.println("User4 Result: " + result5a);\n        System.out.println("User5 Result: " + result5b);
    }
}
