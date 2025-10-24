import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class Task54 {

    // In-memory storage for user credentials (username -> password)
    private static final Map<String, String> users = new HashMap<>();
    // In-memory storage for generated OTPs (username -> otp)
    private static final Map<String, String> userOtps = new HashMap<>();

    // Static initializer to populate user data
    static {
        users.put("user1", "pass123");
        users.put("user2", "secret456");
        users.put("user3", "qwerty");
    }

    /**
     * Verifies the user's primary credentials (username and password).
     * @param username The username.
     * @param password The password.
     * @return true if credentials are valid, false otherwise.
     */
    public static boolean verifyUser(String username, String password) {
        String storedPassword = users.get(username);
        return storedPassword != null && storedPassword.equals(password);
    }

    /**
     * Generates a random 6-digit OTP, stores it, and returns it.
     * In a real application, this would send the OTP via SMS/email.
     * @param username The username for whom to generate the OTP.
     * @return The generated 6-digit OTP as a String.
     */
    public static String generateAndSendOtp(String username) {
        if (users.containsKey(username)) {
            Random random = new Random();
            // Generate a number between 100000 and 999999
            int otpValue = 100000 + random.nextInt(900000);
            String otp = String.valueOf(otpValue);
            userOtps.put(username, otp);
            // Simulate sending OTP to the user
            System.out.println("OTP sent for user '" + username + "'. OTP is: " + otp + " (for simulation purposes)");
            return otp;
        }
        return null;
    }

    /**
     * Verifies the entered OTP against the stored OTP for a user.
     * @param username The username.
     * @param enteredOtp The OTP entered by the user.
     * @return true if the OTP is correct, false otherwise.
     */
    public static boolean verifyOtp(String username, String enteredOtp) {
        String storedOtp = userOtps.get(username);
        if (storedOtp != null && storedOtp.equals(enteredOtp)) {
            // OTP is single-use, remove it after verification
            userOtps.remove(username);
            return true;
        }
        return false;
    }

    // Main method with test cases
    public static void main(String[] args) {
        // --- Test Case 1: Successful Login ---
        System.out.println("--- Test Case 1: Successful Login ---");
        String user1 = "user1";
        String pass1 = "pass123";
        if (verifyUser(user1, pass1)) {
            System.out.println("Primary authentication successful for " + user1);
            String otp1 = generateAndSendOtp(user1);
            // Simulate user entering the correct OTP
            if (verifyOtp(user1, otp1)) {
                System.out.println("2FA successful. Login complete for " + user1);
            } else {
                System.out.println("2FA failed. Invalid OTP.");
            }
        } else {
            System.out.println("Primary authentication failed for " + user1);
        }
        System.out.println("\n----------------------------------------\n");


        // --- Test Case 2: Incorrect Password ---
        System.out.println("--- Test Case 2: Incorrect Password ---");
        String user2 = "user2";
        String pass2_wrong = "wrongpassword";
        if (verifyUser(user2, pass2_wrong)) {
             System.out.println("Primary authentication successful for " + user2);
             generateAndSendOtp(user2);
        } else {
            System.out.println("Primary authentication failed for " + user2 + ". Incorrect username or password.");
        }
        System.out.println("\n----------------------------------------\n");


        // --- Test Case 3: Incorrect Username ---
        System.out.println("--- Test Case 3: Incorrect Username ---");
        String user3_wrong = "nonexistentuser";
        String pass3 = "qwerty";
        if (verifyUser(user3_wrong, pass3)) {
             System.out.println("Primary authentication successful for " + user3_wrong);
             generateAndSendOtp(user3_wrong);
        } else {
            System.out.println("Primary authentication failed for " + user3_wrong + ". Incorrect username or password.");
        }
        System.out.println("\n----------------------------------------\n");


        // --- Test Case 4: Correct Password, Incorrect OTP ---
        System.out.println("--- Test Case 4: Correct Password, Incorrect OTP ---");
        String user4 = "user3";
        String pass4 = "qwerty";
        if (verifyUser(user4, pass4)) {
            System.out.println("Primary authentication successful for " + user4);
            generateAndSendOtp(user4);
            // Simulate user entering a wrong OTP
            String wrongOtp = "000000";
            System.out.println("User '" + user4 + "' enters OTP: " + wrongOtp);
            if (verifyOtp(user4, wrongOtp)) {
                System.out.println("2FA successful. Login complete for " + user4);
            } else {
                System.out.println("2FA failed. Invalid OTP for " + user4);
            }
        } else {
            System.out.println("Primary authentication failed for " + user4);
        }
        System.out.println("\n----------------------------------------\n");


        // --- Test Case 5: Another Successful Login ---
        System.out.println("--- Test Case 5: Another Successful Login ---");
        String user5 = "user2";
        String pass5 = "secret456";
        if (verifyUser(user5, pass5)) {
            System.out.println("Primary authentication successful for " + user5);
            String otp5 = generateAndSendOtp(user5);
            // Simulate user entering the correct OTP
            if (verifyOtp(user5, otp5)) {
                System.out.println("2FA successful. Login complete for " + user5);
            } else {
                System.out.println("2FA failed. Invalid OTP.");
            }
        } else {
            System.out.println("Primary authentication failed for " + user5);
        }
        System.out.println("\n----------------------------------------\n");
    }
}