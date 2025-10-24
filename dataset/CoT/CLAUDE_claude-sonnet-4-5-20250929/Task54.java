
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;
import java.time.Instant;

public class Task54 {
    private static final int OTP_LENGTH = 6;
    private static final int OTP_VALIDITY_SECONDS = 300; // 5 minutes
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, OTPData> otpStorage = new HashMap<>();
    private static final Map<String, String> userCredentials = new HashMap<>();
    
    static class OTPData {
        String hashedOTP;
        long expiryTime;
        int attempts;
        
        OTPData(String hashedOTP, long expiryTime) {
            this.hashedOTP = hashedOTP;
            this.expiryTime = expiryTime;
            this.attempts = 0;
        }
    }
    
    public static String generateOTP() {
        StringBuilder otp = new StringBuilder();
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp.append(secureRandom.nextInt(10));
        }
        return otp.toString();
    }
    
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            throw new IllegalArgumentException("Invalid username or password");
        }
        String hashedPassword = hashPassword(password);
        userCredentials.put(username, hashedPassword);
    }
    
    public static boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        String storedHash = userCredentials.get(username);
        if (storedHash == null) {
            return false;
        }
        String inputHash = hashPassword(password);
        return storedHash.equals(inputHash);
    }
    
    public static String sendOTP(String username) {
        if (!userCredentials.containsKey(username)) {
            throw new IllegalArgumentException("User not found");
        }
        
        String otp = generateOTP();
        String hashedOTP = hashPassword(otp);
        long expiryTime = Instant.now().getEpochSecond() + OTP_VALIDITY_SECONDS;
        
        otpStorage.put(username, new OTPData(hashedOTP, expiryTime));
        
        return otp;
    }
    
    public static boolean verifyOTP(String username, String otp) {
        if (username == null || otp == null) {
            return false;
        }
        
        OTPData otpData = otpStorage.get(username);
        if (otpData == null) {
            return false;
        }
        
        if (otpData.attempts >= 3) {
            otpStorage.remove(username);
            return false;
        }
        
        otpData.attempts++;
        
        long currentTime = Instant.now().getEpochSecond();
        if (currentTime > otpData.expiryTime) {
            otpStorage.remove(username);
            return false;
        }
        
        String hashedInputOTP = hashPassword(otp);
        boolean isValid = otpData.hashedOTP.equals(hashedInputOTP);
        
        if (isValid) {
            otpStorage.remove(username);
        }
        
        return isValid;
    }
    
    public static boolean twoFactorLogin(String username, String password, String otp) {
        if (!authenticateUser(username, password)) {
            return false;
        }
        return verifyOTP(username, otp);
    }
    
    public static void main(String[] args) {
        System.out.println("Two-Factor Authentication System Test Cases\\n");
        
        // Test Case 1: Successful 2FA login
        System.out.println("Test Case 1: Successful 2FA login");
        registerUser("user1", "SecurePass123");
        String otp1 = sendOTP("user1");
        boolean result1 = twoFactorLogin("user1", "SecurePass123", otp1);
        System.out.println("Result: " + (result1 ? "SUCCESS" : "FAILED"));
        System.out.println();
        
        // Test Case 2: Wrong password
        System.out.println("Test Case 2: Wrong password");
        registerUser("user2", "SecurePass456");
        String otp2 = sendOTP("user2");
        boolean result2 = twoFactorLogin("user2", "WrongPassword", otp2);
        System.out.println("Result: " + (result2 ? "SUCCESS" : "FAILED (Expected)"));
        System.out.println();
        
        // Test Case 3: Wrong OTP
        System.out.println("Test Case 3: Wrong OTP");
        registerUser("user3", "SecurePass789");
        sendOTP("user3");
        boolean result3 = twoFactorLogin("user3", "SecurePass789", "000000");
        System.out.println("Result: " + (result3 ? "SUCCESS" : "FAILED (Expected)"));
        System.out.println();
        
        // Test Case 4: OTP expiry simulation
        System.out.println("Test Case 4: Valid OTP verification");
        registerUser("user4", "SecurePass101");
        String otp4 = sendOTP("user4");
        boolean result4 = verifyOTP("user4", otp4);
        System.out.println("Result: " + (result4 ? "SUCCESS" : "FAILED"));
        System.out.println();
        
        // Test Case 5: Multiple users
        System.out.println("Test Case 5: Multiple users handling");
        registerUser("user5", "SecurePass202");
        registerUser("user6", "SecurePass303");
        String otp5 = sendOTP("user5");
        String otp6 = sendOTP("user6");
        boolean result5a = twoFactorLogin("user5", "SecurePass202", otp5);
        boolean result5b = twoFactorLogin("user6", "SecurePass303", otp6);
        System.out.println("User5 Result: " + (result5a ? "SUCCESS" : "FAILED"));
        System.out.println("User6 Result: " + (result5b ? "SUCCESS" : "FAILED"));
    }
}
