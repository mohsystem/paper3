import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Task54 {

    // In-memory storage for user credentials.
    // WARNING: Storing plain-text passwords is insecure. Use hashed passwords in production.
    private static final Map<String, String> userDatabase = new HashMap<>();
    // In-memory storage for one-time passwords.
    private static final Map<String, String> otpStorage = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();

    static {
        // Populate the user database
        userDatabase.put("alice", "password123");
        userDatabase.put("bob", "bob@1234");
    }

    /**
     * Factor 1: Validates username and password.
     *
     * @param username The user's username.
     * @param password The user's password.
     * @return true if credentials are valid, false otherwise.
     */
    public static boolean login(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        String storedPassword = userDatabase.get(username);
        return storedPassword != null && storedPassword.equals(password);
    }

    /**
     * Generates and "sends" a 6-digit OTP for the user.
     * In a real application, this would be sent via SMS or email.
     *
     * @param username The user to generate an OTP for.
     * @return The generated OTP.
     */
    public static String generateAndSendOTP(String username) {
        if (username == null || !userDatabase.containsKey(username)) {
            return null;
        }
        // Generate a 6-digit OTP (100000 to 999999)
        int otpValue = 100000 + secureRandom.nextInt(900000);
        String otpString = String.valueOf(otpValue);
        
        // Store the OTP for validation
        otpStorage.put(username, otpString);
        
        // Simulate sending the OTP to the user
        System.out.println("OTP sent to " + username + ". Your OTP is: " + otpString);
        return otpString;
    }

    /**
     * Factor 2: Validates the provided OTP for the user.
     *
     * @param username The user's username.
     * @param otp The one-time password provided by the user.
     * @return true if the OTP is valid, false otherwise.
     */
    public static boolean validateOTP(String username, String otp) {
        if (username == null || otp == null) {
            return false;
        }
        String storedOtp = otpStorage.get(username);
        if (storedOtp != null && storedOtp.equals(otp)) {
            // Invalidate the OTP after use
            otpStorage.remove(username);
            return true;
        }
        return false;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running 2FA Login Test Cases ---");

        // Test Case 1: Successful Login
        System.out.println("\n--- Test Case 1: Successful Login ---");
        String user1 = "alice";
        String pass1 = "password123";
        System.out.println("Attempting to log in user: " + user1);
        if (login(user1, pass1)) {
            System.out.println("Step 1 (Password) successful.");
            String generatedOtp1 = generateAndSendOTP(user1);
            // Simulate user inputting the correct OTP
            if (validateOTP(user1, generatedOtp1)) {
                System.out.println("Step 2 (OTP) successful. Login successful for " + user1 + "!");
            } else {
                System.out.println("Step 2 (OTP) failed. Login failed.");
            }
        } else {
            System.out.println("Step 1 (Password) failed. Invalid username or password.");
        }

        // Test Case 2: Invalid Password
        System.out.println("\n--- Test Case 2: Invalid Password ---");
        String user2 = "bob";
        String pass2 = "wrongpassword";
        System.out.println("Attempting to log in user: " + user2);
        if (login(user2, pass2)) {
            System.out.println("Step 1 (Password) successful.");
            generateAndSendOTP(user2);
        } else {
            System.out.println("Step 1 (Password) failed. Invalid username or password.");
        }

        // Test Case 3: Invalid Username
        System.out.println("\n--- Test Case 3: Invalid Username ---");
        String user3 = "charlie";
        String pass3 = "password123";
        System.out.println("Attempting to log in user: " + user3);
        if (login(user3, pass3)) {
            System.out.println("Step 1 (Password) successful.");
        } else {
            System.out.println("Step 1 (Password) failed. Invalid username or password.");
        }

        // Test Case 4: Incorrect OTP
        System.out.println("\n--- Test Case 4: Incorrect OTP ---");
        String user4 = "bob";
        String pass4 = "bob@1234";
        System.out.println("Attempting to log in user: " + user4);
        if (login(user4, pass4)) {
            System.out.println("Step 1 (Password) successful.");
            generateAndSendOTP(user4);
            // Simulate user inputting an incorrect OTP
            String incorrectOtp = "111111";
            System.out.println("User provides OTP: " + incorrectOtp);
            if (validateOTP(user4, incorrectOtp)) {
                System.out.println("Step 2 (OTP) successful. Login successful for " + user4 + "!");
            } else {
                System.out.println("Step 2 (OTP) failed. Login failed.");
            }
        } else {
            System.out.println("Step 1 (Password) failed. Invalid username or password.");
        }
        
        // Test Case 5: Full successful flow for the second user
        System.out.println("\n--- Test Case 5: Another Successful Login ---");
        String user5 = "bob";
        String pass5 = "bob@1234";
        System.out.println("Attempting to log in user: " + user5);
        if (login(user5, pass5)) {
            System.out.println("Step 1 (Password) successful.");
            String generatedOtp5 = generateAndSendOTP(user5);
            if (validateOTP(user5, generatedOtp5)) {
                System.out.println("Step 2 (OTP) successful. Login successful for " + user5 + "!");
            } else {
                System.out.println("Step 2 (OTP) failed. Login failed.");
            }
        } else {
            System.out.println("Step 1 (Password) failed. Invalid username or password.");
        }
    }
}