import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task54 {

    private static final int OTP_LENGTH = 6;
    private static final long OTP_VALIDITY_SECONDS = 60;
    private static final Map<String, OtpData> otpStore = new ConcurrentHashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();

    private record OtpData(String otp, Instant timestamp) {}

    /**
     * Generates a random One-Time Password (OTP) for a user.
     * In a real application, this would also trigger sending the OTP to the user
     * via SMS, email, etc.
     *
     * @param username The username for whom to generate the OTP.
     * @return The generated 6-digit OTP string.
     */
    public static String generateOtp(String username) {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty.");
        }
        int max = (int) Math.pow(10, OTP_LENGTH);
        int otpInt = secureRandom.nextInt(max);
        String otp = String.format("%0" + OTP_LENGTH + "d", otpInt);

        otpStore.put(username, new OtpData(otp, Instant.now()));
        return otp;
    }

    /**
     * Verifies the submitted OTP for a given user.
     *
     * @param username The username to verify.
     * @param submittedOtp The OTP submitted by the user.
     * @return true if the OTP is valid, false otherwise.
     */
    public static boolean verifyOtp(String username, String submittedOtp) {
        if (username == null || submittedOtp == null) {
            return false;
        }

        OtpData otpData = otpStore.get(username);
        
        // Invalidate OTP after first use by removing it
        if (otpData != null) {
            otpStore.remove(username);
        } else {
            return false;
        }

        // Check for expiration
        long secondsElapsed = Duration.between(otpData.timestamp(), Instant.now()).getSeconds();
        if (secondsElapsed > OTP_VALIDITY_SECONDS) {
            return false;
        }

        // Constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(otpData.otp().getBytes(), submittedOtp.getBytes());
    }

    public static void main(String[] args) {
        String testUser = "testuser@example.com";

        // Test Case 1: Successful verification
        System.out.println("--- Test Case 1: Successful Verification ---");
        String otp1 = generateOtp(testUser);
        System.out.println("Generated OTP for " + testUser + ": " + otp1 + " (for testing)");
        boolean isValid1 = verifyOtp(testUser, otp1);
        System.out.println("Verification successful: " + isValid1);
        System.out.println();

        // Test Case 2: Failed verification due to incorrect OTP
        System.out.println("--- Test Case 2: Incorrect OTP ---");
        String otp2 = generateOtp(testUser);
        System.out.println("Generated OTP for " + testUser + ": " + otp2 + " (for testing)");
        String wrongOtp = "000000";
        boolean isValid2 = verifyOtp(testUser, wrongOtp);
        System.out.println("Verification with wrong OTP (" + wrongOtp + ") successful: " + isValid2);
        System.out.println();

        // Test Case 3: Failed verification due to reuse
        System.out.println("--- Test Case 3: OTP Reuse ---");
        String otp3 = generateOtp(testUser);
        System.out.println("Generated OTP for " + testUser + ": " + otp3 + " (for testing)");
        verifyOtp(testUser, otp3); // First, successful use
        System.out.println("Attempting to reuse OTP " + otp3);
        boolean isValid3 = verifyOtp(testUser, otp3); // Second, failed use
        System.out.println("Second verification successful: " + isValid3);
        System.out.println();

        // Test Case 4: Failed verification due to expiration
        System.out.println("--- Test Case 4: Expired OTP ---");
        long shortValidity = 1; // 1 second validity for test
        // We cannot modify the constant, so we will simulate waiting
        String otp4 = generateOtp(testUser);
        System.out.println("Generated OTP for " + testUser + ": " + otp4 + " (for testing)");
        System.out.println("Waiting for " + (OTP_VALIDITY_SECONDS + 1) + " seconds to simulate expiration...");
        try {
            TimeUnit.SECONDS.sleep(OTP_VALIDITY_SECONDS + 1);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        boolean isValid4 = verifyOtp(testUser, otp4);
        System.out.println("Verification of expired OTP successful: " + isValid4);
        System.out.println();

        // Test Case 5: Verification for a user with no generated OTP
        System.out.println("--- Test Case 5: No OTP Generated for User ---");
        boolean isValid5 = verifyOtp("nouser@example.com", "123456");
        System.out.println("Verification for non-existent OTP successful: " + isValid5);
        System.out.println();
    }
}