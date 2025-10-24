import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Task54 {

    // IMPORTANT: In a real-world application, never store passwords in plaintext.
    // They should be securely hashed and salted. This is for demonstration purposes only.
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        userDatabase.put("alice", "password123");
        userDatabase.put("bob", "bob@secret");
    }

    /**
     * Verifies user credentials against the stored database.
     * @param username The username to check.
     * @param password The password to check.
     * @return true if credentials are valid, false otherwise.
     */
    public static boolean verifyCredentials(String username, String password) {
        String storedPassword = userDatabase.get(username);
        return storedPassword != null && storedPassword.equals(password);
    }

    /**
     * Generates a cryptographically secure 6-digit One-Time Password (OTP).
     * @return A 6-digit OTP as a String.
     */
    public static String generateOTP() {
        SecureRandom random = new SecureRandom();
        // Generate a number between 100000 and 999999
        int otp = 100000 + random.nextInt(900000);
        return String.valueOf(otp);
    }

    /**
     * Verifies if the user-provided OTP matches the generated OTP.
     * @param generatedOTP The system-generated OTP.
     * @param userInputOTP The OTP entered by the user.
     * @return true if the OTPs match, false otherwise.
     */
    public static boolean verifyOTP(String generatedOTP, String userInputOTP) {
        return generatedOTP != null && generatedOTP.equals(userInputOTP);
    }

    /**
     * Simulates the full login process for a given user.
     * @param username The user's username.
     * @param password The user's password.
     * @param otpInput The OTP provided by the user for the simulation.
     */
    public static void loginProcess(String username, String password, String otpInput) {
        System.out.println("--- Attempting login for user: " + username + " ---");
        if (verifyCredentials(username, password)) {
            System.out.println("Credentials verified. Generating OTP...");
            String otp = generateOTP();
            
            // In a real application, this OTP would be sent to the user via SMS, email, etc.
            // For this simulation, we will print it to the console.
            System.out.println("Generated OTP (for simulation): " + otp);
            System.out.println("User provided OTP: " + otpInput);

            if (verifyOTP(otp, otpInput)) {
                System.out.println("OTP verification successful. Login successful!");
            } else {
                System.out.println("OTP verification failed. Access denied.");
            }
        } else {
            System.out.println("Invalid username or password. Access denied.");
        }
        System.out.println("-----------------------------------------\n");
    }

    public static void main(String[] args) {
        // Test Case 1: Successful login
        // Simulate correct password and then providing the correct OTP.
        // For the test, we'll "peek" at the OTP to simulate a user receiving and entering it.
        String correctUsername1 = "alice";
        String correctPassword1 = "password123";
        if (verifyCredentials(correctUsername1, correctPassword1)) {
            String generatedOtp1 = generateOTP(); // We generate it to pass it to the simulation
            loginProcess(correctUsername1, correctPassword1, generatedOtp1);
        }

        // Test Case 2: Incorrect password
        loginProcess("alice", "wrongpassword", "123456");

        // Test Case 3: Correct password, incorrect OTP
        loginProcess("bob", "bob@secret", "000000");

        // Test Case 4: Non-existent user
        loginProcess("charlie", "somepassword", "654321");
        
        // Test Case 5: Another successful login
        String correctUsername2 = "bob";
        String correctPassword2 = "bob@secret";
        if (verifyCredentials(correctUsername2, correctPassword2)) {
            String generatedOtp2 = generateOTP();
            loginProcess(correctUsername2, correctPassword2, generatedOtp2);
        }
    }
}