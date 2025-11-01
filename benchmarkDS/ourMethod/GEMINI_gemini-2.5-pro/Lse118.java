import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.security.SecureRandom;
import java.util.stream.Stream;

public class Lse118 {

    /**
     * Generates 3 random floats, concatenates them into a string, and writes the string to a new file.
     *
     * @param fileName The name of the file to create. Must not contain path separators.
     * @return true if the file was written successfully, false otherwise.
     */
    public static boolean generateAndWriteRandomFloats(String fileName) {
        if (fileName == null || fileName.isEmpty()) {
            System.err.println("File name cannot be null or empty.");
            return false;
        }

        Path path = Paths.get(fileName);
        // Rule #1: Prevent path traversal by ensuring the filename does not contain directory components.
        if (path.getParent() != null) {
            System.err.println("Invalid file name: Path components are not allowed.");
            return false;
        }

        SecureRandom random = new SecureRandom();
        StringBuilder sb = new StringBuilder();

        // 1. Generate 3 random floats
        // 2. Convert them to strings
        // 3. Concatenate them
        for (int i = 0; i < 3; i++) {
            float r = random.nextFloat();
            sb.append(Float.toString(r));
        }

        String content = sb.toString();

        // 4. Write the string to a file using secure patterns.
        // Rules #2, #3, #4: Use try-with-resources and atomic/secure file creation options.
        // CREATE_NEW fails if the file already exists, preventing overwrites and TOCTOU races.
        try (BufferedWriter writer = Files.newBufferedWriter(path, StandardCharsets.UTF_8,
                                                              StandardOpenOption.WRITE,
                                                              StandardOpenOption.CREATE_NEW)) {
            writer.write(content);
        } catch (java.nio.file.FileAlreadyExistsException e) {
            System.err.println("Error: File already exists: " + path);
            return false;
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Security error creating file: " + e.getMessage());
            return false;
        }
        
        return true;
    }

    /**
     * Main method with 5 test cases.
     */
    public static void main(String[] args) {
        System.out.println("Running Java test cases...");
        String[] testFiles = {
            "java_test1.txt", 
            "java_test2.txt", 
            "java_test3.txt", 
            "java_test4.txt", 
            "java_test5.txt"
        };

        for (String fileName : testFiles) {
            // Clean up file before test if it exists
            try {
                Files.deleteIfExists(Paths.get(fileName));
            } catch (IOException e) {
                // Ignore cleanup errors, test will likely fail with a clear message anyway
            }

            System.out.println("Attempting to write to: " + fileName);
            if (generateAndWriteRandomFloats(fileName)) {
                System.out.println("Successfully wrote to " + fileName);
                try {
                    String content = Files.readString(Paths.get(fileName));
                    System.out.println("File content: " + content);
                } catch (IOException e) {
                    System.err.println("Failed to read back file content: " + e.getMessage());
                }
            } else {
                System.err.println("Failed to write to " + fileName);
            }
            System.out.println("---");
        }
        
        // Test case with an invalid path to demonstrate path traversal prevention
        System.out.println("Testing invalid path...");
        String invalidPath = Paths.get("..", "invalid_test.txt").toString();
        if (!generateAndWriteRandomFloats(invalidPath)) {
            System.out.println("Correctly failed to write to invalid path: " + invalidPath);
        } else {
            System.err.println("Incorrectly allowed writing to invalid path: " + invalidPath);
        }
    }
}