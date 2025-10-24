
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Task54 {
    private static final Map<String, String> userDatabase = new HashMap<>();
    private static final Map<String, String> otpStorage = new HashMap<>();
    private static final Map<String, Long> otpExpiry = new HashMap<>();
    private static final int OTP_LENGTH = 6;
    private static final long OTP_VALIDITY = 300000; // 5 minutes in milliseconds
    private static final SecureRandom secureRandom = new SecureRandom();

    public static String generateOTP() {
        StringBuilder otp = new StringBuilder();
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp.append(secureRandom.nextInt(10));
        }
        return otp.toString();
    }

    public static boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }
        if (userDatabase.containsKey(username)) {
            return false;
        }
        userDatabase.put(username, password);
        return true;
    }

    public static String authenticateAndGenerateOTP(String username, String password) {
        if (username == null || password == null) {
            return null;
        }
        if (!userDatabase.containsKey(username)) {
            return null;
        }
        if (!userDatabase.get(username).equals(password)) {
            return null;
        }
        String otp = generateOTP();
        otpStorage.put(username, otp);
        otpExpiry.put(username, System.currentTimeMillis() + OTP_VALIDITY);
        return otp;
    }

    public static boolean verifyOTP(String username, String otp) {
        if (username == null || otp == null) {
            return false;
        }
        if (!otpStorage.containsKey(username)) {
            return false;
        }
        if (System.currentTimeMillis() > otpExpiry.get(username)) {
            otpStorage.remove(username);
            otpExpiry.remove(username);
            return false;
        }
        boolean isValid = otpStorage.get(username).equals(otp);
        if (isValid) {
            otpStorage.remove(username);
            otpExpiry.remove(username);
        }
        return isValid;
    }

    public static void main(String[] args) {
        System.out.println("=== Two-Factor Authentication Test Cases ===\\n");

        // Test Case 1: Register and successful 2FA
        System.out.println("Test Case 1: Successful Registration and 2FA");
        registerUser("user1", "password123");
        String otp1 = authenticateAndGenerateOTP("user1", "password123");
        System.out.println("Generated OTP: " + otp1);
        boolean result1 = verifyOTP("user1", otp1);
        System.out.println("2FA Result: " + (result1 ? "SUCCESS" : "FAILED"));
        System.out.println();

        // Test Case 2: Invalid password
        System.out.println("Test Case 2: Invalid Password");
        registerUser("user2", "securepass");
        String otp2 = authenticateAndGenerateOTP("user2", "wrongpass");
        System.out.println("OTP Generation: " + (otp2 == null ? "FAILED (Invalid credentials)" : "SUCCESS"));
        System.out.println();

        // Test Case 3: Wrong OTP
        System.out.println("Test Case 3: Wrong OTP");
        registerUser("user3", "mypassword");
        String otp3 = authenticateAndGenerateOTP("user3", "mypassword");
        System.out.println("Generated OTP: " + otp3);
        boolean result3 = verifyOTP("user3", "000000");
        System.out.println("2FA with wrong OTP: " + (result3 ? "SUCCESS" : "FAILED"));
        System.out.println();

        // Test Case 4: Reusing OTP
        System.out.println("Test Case 4: Reusing OTP");
        registerUser("user4", "pass456");
        String otp4 = authenticateAndGenerateOTP("user4", "pass456");
        System.out.println("Generated OTP: " + otp4);
        boolean result4a = verifyOTP("user4", otp4);
        System.out.println("First verification: " + (result4a ? "SUCCESS" : "FAILED"));
        boolean result4b = verifyOTP("user4", otp4);
        System.out.println("Second verification (reuse): " + (result4b ? "SUCCESS" : "FAILED"));
        System.out.println();

        // Test Case 5: Non-existent user
        System.out.println("Test Case 5: Non-existent User");
        String otp5 = authenticateAndGenerateOTP("nonexistent", "password");
        System.out.println("OTP Generation for non-existent user: " + (otp5 == null ? "FAILED (User not found)" : "SUCCESS"));
        System.out.println();
    }
}
