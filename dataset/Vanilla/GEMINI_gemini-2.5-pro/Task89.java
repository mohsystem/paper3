import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

public class Task89 {

    /**
     * Creates a dummy file with the given password.
     * @param filename The name of the file to create.
     * @param password The password to store in the file.
     */
    public static void createPasswordFile(String filename, String password) {
        try {
            Files.write(Paths.get(filename), password.getBytes());
        } catch (IOException e) {
            System.err.println("Error creating password file: " + e.getMessage());
        }
    }

    /**
     * Compares the password in a file with the user-provided password
     * by checking each character sequentially.
     * @param filename The name of the file containing the stored password.
     * @param userInput The password entered by the user.
     * @return true if the passwords match, false otherwise.
     */
    public static boolean comparePassword(String filename, String userInput) {
        try {
            // Read the stored password from the file
            String storedPassword = new String(Files.readAllBytes(Paths.get(filename)));
            
            // First, check if the lengths are the same. If not, they can't match.
            if (storedPassword.length() != userInput.length()) {
                return false;
            }

            // Compare each character sequentially
            for (int i = 0; i < storedPassword.length(); i++) {
                if (storedPassword.charAt(i) != userInput.charAt(i)) {
                    return false; // Mismatch found
                }
            }

            // If the loop completes, all characters matched
            return true;

        } catch (IOException e) {
            System.err.println("Error reading password file: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String filename = "password.txt";
        String correctPassword = "P@ssw0rd123";

        // Create the password file for testing
        createPasswordFile(filename, correctPassword);

        // --- Test Cases ---
        String[] testPasswords = {
            "P@ssw0rd123",    // 1. Correct password
            "p@ssw0rd123",    // 2. Incorrect case
            "P@ssw0rd",       // 3. Incorrect length (shorter)
            "P@ssw0rd124",    // 4. Same length, different character
            ""                // 5. Empty password
        };

        System.out.println("--- Java Test Cases ---");
        for (String test : testPasswords) {
            boolean isMatch = comparePassword(filename, test);
            System.out.println("Testing with \"" + test + "\": " + (isMatch ? "Match" : "No Match"));
        }

        // Clean up the created file
        new File(filename).delete();
    }
}