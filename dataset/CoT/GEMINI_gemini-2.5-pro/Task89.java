import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Paths;

class Task89 {

    /**
     * WARNING: Storing passwords in plaintext is extremely insecure. 
     * In a real-world application, you should always use a strong, salted,
     * one-way hashing algorithm like Argon2, scrypt, or bcrypt to store password hashes,
     * not the passwords themselves. This implementation is for demonstration purposes only.
     *
     * Compares a user-provided password with one stored in a file using a
     * constant-time comparison algorithm to mitigate timing attacks.
     *
     * @param filename The path to the file containing the stored password.
     * @param userInput The password entered by the user.
     * @return true if the passwords match, false otherwise.
     */
    public static boolean comparePassword(String filename, String userInput) {
        String storedPassword = "";
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            // Read the first line from the file which contains the password
            storedPassword = reader.readLine();
        } catch (IOException e) {
            System.err.println("Error reading password file: " + e.getMessage());
            return false;
        }

        if (storedPassword == null) {
            storedPassword = ""; // Treat missing password as an empty string
        }
        
        // Convert strings to char arrays for comparison
        char[] storedChars = storedPassword.toCharArray();
        char[] userChars = userInput.toCharArray();

        // Length check is performed first. Note: This can leak password length information.
        if (storedChars.length != userChars.length) {
            return false;
        }

        // Constant-time comparison to prevent timing attacks.
        // We iterate through all characters regardless of when a mismatch is found.
        int diff = 0;
        for (int i = 0; i < storedChars.length; i++) {
            // XOR the characters. If they are the same, the result is 0.
            // OR the result into diff. If any character is different, diff will become non-zero.
            diff |= storedChars[i] ^ userChars[i];
        }

        // If diff is 0, it means all characters were identical.
        return diff == 0;
    }

    public static void main(String[] args) {
        String testFilename = "password.txt";
        String correctPassword = "S3cur3P@ssw0rd!";

        // Setup: Create a dummy password file for testing
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(testFilename))) {
            writer.write(correctPassword);
        } catch (IOException e) {
            System.err.println("Failed to create test password file.");
            return;
        }

        System.out.println("--- Java Test Cases ---");
        // Test Case 1: Correct password
        boolean result1 = comparePassword(testFilename, "S3cur3P@ssw0rd!");
        System.out.println("Test 1 (Correct): " + (result1 ? "PASS" : "FAIL"));

        // Test Case 2: Incorrect password (different first character)
        boolean result2 = comparePassword(testFilename, "X3cur3P@ssw0rd!");
        System.out.println("Test 2 (Incorrect, first char): " + (!result2 ? "PASS" : "FAIL"));

        // Test Case 3: Incorrect password (different last character)
        boolean result3 = comparePassword(testFilename, "S3cur3P@ssw0rd?");
        System.out.println("Test 3 (Incorrect, last char): " + (!result3 ? "PASS" : "FAIL"));

        // Test Case 4: Incorrect password (shorter)
        boolean result4 = comparePassword(testFilename, "S3cur3");
        System.out.println("Test 4 (Incorrect, shorter): " + (!result4 ? "PASS" : "FAIL"));

        // Test Case 5: Incorrect password (longer)
        boolean result5 = comparePassword(testFilename, "S3cur3P@ssw0rd!Extra");
        System.out.println("Test 5 (Incorrect, longer): " + (!result5 ? "PASS" : "FAIL"));

        // Cleanup: Delete the test file
        try {
            Files.delete(Paths.get(testFilename));
        } catch (IOException e) {
            System.err.println("Failed to delete test file.");
        }
    }
}