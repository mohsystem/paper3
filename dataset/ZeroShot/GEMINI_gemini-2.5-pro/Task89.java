import java.security.MessageDigest;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;

// Note: Storing plaintext passwords is a major security risk.
// In a real-world application, you should store a strong cryptographic hash
// of the password (e.g., using Argon2, scrypt, or bcrypt) and compare the hashes.
// The prompt describes an insecure, sequential comparison which is vulnerable
// to timing attacks. This code implements a secure, constant-time comparison.

public class Task89 {

    /**
     * Securely compares two passwords in a way that prevents timing attacks.
     * Java's MessageDigest.isEqual is a built-in function that performs a
     * constant-time comparison, making it ideal for this purpose.
     *
     * @param userInputPassword The password entered by the user.
     * @param storedPassword    The password that would be read from a file or database.
     * @return true if the passwords match, false otherwise.
     */
    public static boolean securePasswordCompare(String userInputPassword, String storedPassword) {
        // Handle null inputs to prevent NullPointerExceptions.
        if (userInputPassword == null || storedPassword == null) {
            return false;
        }

        // Convert strings to byte arrays for comparison. Using a consistent
        // encoding like UTF-8 is important.
        byte[] userBytes = userInputPassword.getBytes(StandardCharsets.UTF_8);
        byte[] storedBytes = storedPassword.getBytes(StandardCharsets.UTF_8);

        // MessageDigest.isEqual performs a constant-time comparison.
        // It processes every byte regardless of when a mismatch is found,
        // thus preventing timing analysis from revealing information.
        return MessageDigest.isEqual(userBytes, storedBytes);
    }

    public static void main(String[] args) {
        // This would be the password stored securely (as a hash in a real app).
        String storedPassword = "S3cureP@ssw0rd!23";

        System.out.println("--- Java: Test Cases for Secure Password Comparison ---");

        // Test Case 1: Correct password
        String input1 = "S3cureP@ssw0rd!23";
        boolean result1 = securePasswordCompare(input1, storedPassword);
        System.out.println("Test 1 (Correct Match): " + (result1 ? "PASS" : "FAIL"));

        // Test Case 2: Incorrect password (different case)
        String input2 = "s3curep@ssw0rd!23";
        boolean result2 = securePasswordCompare(input2, storedPassword);
        System.out.println("Test 2 (Case Mismatch): " + (!result2 ? "PASS" : "FAIL"));

        // Test Case 3: Incorrect password (different length)
        String input3 = "S3cureP@ssw0rd!";
        boolean result3 = securePasswordCompare(input3, storedPassword);
        System.out.println("Test 3 (Length Mismatch): " + (!result3 ? "PASS" : "FAIL"));

        // Test Case 4: Incorrect password (same length, different char)
        String input4 = "S3cureP@ssw0rd!24";
        boolean result4 = securePasswordCompare(input4, storedPassword);
        System.out.println("Test 4 (Character Mismatch): " + (!result4 ? "PASS" : "FAIL"));

        // Test Case 5: Empty string vs stored password
        String input5 = "";
        boolean result5 = securePasswordCompare(input5, storedPassword);
        System.out.println("Test 5 (Empty Input): " + (!result5 ? "PASS" : "FAIL"));
    }
}