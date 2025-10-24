import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.charset.StandardCharsets;

public class Task65 {

    /**
     * Reads the content of a file after validating the filename.
     *
     * @param filename The name of the file to read.
     * @return The content of the file as a String, or null if an error occurs.
     */
    public static String readFileContent(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return null;
        }

        // Security: Prevent path traversal attacks.
        if (filename.contains("..")) {
            System.err.println("Error: Invalid filename. Path traversal is not allowed.");
            return null;
        }

        try {
            Path path = Paths.get(filename).toAbsolutePath().normalize();
            
            // Basic validation: check if file exists, is a regular file, and is readable.
            if (!Files.exists(path)) {
                System.err.println("Error: File does not exist: " + path);
                return null;
            }
            if (!Files.isRegularFile(path)) {
                System.err.println("Error: Path is not a regular file: " + path);
                return null;
            }
            if (!Files.isReadable(path)) {
                System.err.println("Error: File is not readable (permission denied): " + path);
                return null;
            }

            // Read all bytes and convert to a string.
            return new String(Files.readAllBytes(path), StandardCharsets.UTF_8);

        } catch (IOException | SecurityException e) {
            System.err.println("Error reading file '" + filename + "': " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return null;
        }
    }

    // Helper method to create a test file
    private static void createTestFile(String filename, String content) {
        try {
            Files.writeString(Paths.get(filename), content, StandardCharsets.UTF_8);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + filename);
        }
    }

    // Helper method to run test cases
    public static void runTests() {
        System.out.println("--- Running Tests ---");

        // Test Case 1: Valid file with content
        System.out.println("\n--- Test Case 1: Read a valid file ---");
        String testFile1 = "test1.txt";
        String content1 = "Hello World!";
        createTestFile(testFile1, content1);
        String result1 = readFileContent(testFile1);
        System.out.println("Expected: " + content1);
        System.out.println("Got: " + result1);
        System.out.println("Test " + (content1.equals(result1) ? "PASSED" : "FAILED"));

        // Test Case 2: Empty file
        System.out.println("\n--- Test Case 2: Read an empty file ---");
        String testFile2 = "test2.txt";
        createTestFile(testFile2, "");
        String result2 = readFileContent(testFile2);
        System.out.println("Expected: ");
        System.out.println("Got: " + result2);
        System.out.println("Test " + ("".equals(result2) ? "PASSED" : "FAILED"));

        // Test Case 3: Non-existent file
        System.out.println("\n--- Test Case 3: Read a non-existent file ---");
        String result3 = readFileContent("nonexistent.txt");
        System.out.println("Expected: null (with error message)");
        System.out.println("Got: " + result3);
        System.out.println("Test " + (result3 == null ? "PASSED" : "FAILED"));


        // Test Case 4: Path traversal attempt
        System.out.println("\n--- Test Case 4: Path traversal attempt ---");
        String result4 = readFileContent("../somefile.txt");
        System.out.println("Expected: null (with error message)");
        System.out.println("Got: " + result4);
        System.out.println("Test " + (result4 == null ? "PASSED" : "FAILED"));

        // Test Case 5: File with multi-line content
        System.out.println("\n--- Test Case 5: Read a multi-line file ---");
        String testFile5 = "test5.txt";
        String content5 = "Line 1\nLine 2\nLine 3";
        createTestFile(testFile5, content5);
        String result5 = readFileContent(testFile5);
        System.out.println("Expected:\n" + content5);
        System.out.println("Got:\n" + result5);
        System.out.println("Test " + (content5.equals(result5) ? "PASSED" : "FAILED"));


        // Cleanup
        try {
            Files.deleteIfExists(Paths.get(testFile1));
            Files.deleteIfExists(Paths.get(testFile2));
            Files.deleteIfExists(Paths.get(testFile5));
        } catch (IOException e) {
            System.err.println("Failed to clean up test files.");
        }
        System.out.println("\n--- Tests Finished ---");
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            String filename = args[0];
            System.out.println("Attempting to read file from command line: " + filename);
            String content = readFileContent(filename);
            if (content != null) {
                System.out.println("\n--- File Content ---");
                System.out.println(content);
                System.out.println("--- End of Content ---");
            } else {
                System.out.println("Failed to read file.");
            }
        } else {
            System.out.println("No filename provided via command line. Running built-in test cases.");
            runTests();
        }
    }
}