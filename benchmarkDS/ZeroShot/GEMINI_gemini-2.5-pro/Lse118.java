import java.io.FileWriter;
import java.io.IOException;
import java.security.SecureRandom;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Lse118 {

    /**
     * Generates 3 cryptographically strong random floats, concatenates them into a space-separated
     * string, and writes the string to a specified file.
     *
     * @param filename The name of the file to write to. It should not be derived from untrusted
     *                 input to prevent path traversal vulnerabilities.
     * @return true on success, false on failure.
     */
    public static boolean generateAndWriteRandoms(String filename) {
        // Use SecureRandom for cryptographically strong random numbers.
        SecureRandom secureRandom = new SecureRandom();

        // Generate 3 random floats. nextFloat() returns a value between 0.0 and 1.0.
        Float f1 = secureRandom.nextFloat();
        Float f2 = secureRandom.nextFloat();
        Float f3 = secureRandom.nextFloat();

        // Convert floats to strings and concatenate them with spaces in between.
        String concatenatedString = Stream.of(f1, f2, f3)
                                          .map(String::valueOf)
                                          .collect(Collectors.joining(" "));

        // Use try-with-resources to ensure the writer is closed automatically.
        // This prevents resource leaks, which is a security best practice.
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(concatenatedString);
            return true;
        } catch (IOException e) {
            // Proper error handling is crucial for security and stability.
            System.err.println("Error writing to file '" + filename + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running Java test cases...");
        for (int i = 1; i <= 5; i++) {
            String testFilename = "output_java_" + i + ".txt";
            System.out.println("Test Case " + i + ": Writing to " + testFilename);
            boolean success = generateAndWriteRandoms(testFilename);
            if (success) {
                System.out.println(" -> Success.");
            } else {
                System.out.println(" -> Failed.");
            }
        }
    }
}