import java.io.IOException;
import java.nio.file.*;
import java.io.File;

public class Lse44 {

    /**
     * Reads a file from the 'safe' directory, preventing path traversal.
     * @param filename The name of the file to read.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFileFromSafeDirectory(String filename) {
        // Use a relative path for the safe directory for testing purposes.
        Path safeDirPath = Paths.get("safe").toAbsolutePath();

        // Basic check for invalid characters.
        if (filename == null || filename.isEmpty() || filename.contains("..") || filename.contains("/") || filename.contains("\\")) {
            return "Error: Invalid filename (contains traversal or path characters).";
        }
        
        Path requestedFilePath = safeDirPath.resolve(filename).normalize();

        // Security check: Ensure the resolved path is still within the safe directory.
        if (!requestedFilePath.startsWith(safeDirPath)) {
            return "Error: Directory traversal attempt detected.";
        }

        if (!Files.exists(requestedFilePath) || Files.isDirectory(requestedFilePath)) {
             return "Error: File not found or is a directory.";
        }

        try {
            return new String(Files.readAllBytes(requestedFilePath));
        } catch (IOException e) {
            return "Error: Could not read file. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Setup for test cases
        try {
            Files.createDirectories(Paths.get("safe"));
            Files.write(Paths.get("safe/test1.txt"), "This is test file 1.".getBytes());
            Files.write(Paths.get("safe/another_file.log"), "Log entry.".getBytes());
            System.out.println("Test environment set up.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // Test cases
        String[] testFiles = {
            "test1.txt",              // 1. Valid file
            "another_file.log",       // 2. Another valid file
            "nonexistent.txt",        // 3. Non-existent file
            "../secret.txt",          // 4. Directory traversal attempt
            "../../etc/passwd"        // 5. More complex traversal attempt
        };

        System.out.println("\n--- Running Test Cases ---");
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": Requesting '" + testFiles[i] + "'");
            String content = readFileFromSafeDirectory(testFiles[i]);
            System.out.println("Result: " + content);
            System.out.println("----------------------------");
        }

        // Cleanup
        try {
            Files.delete(Paths.get("safe/test1.txt"));
            Files.delete(Paths.get("safe/another_file.log"));
            Files.delete(Paths.get("safe"));
            System.out.println("\nTest environment cleaned up.");
        } catch (IOException e) {
            System.err.println("Failed to clean up test environment: " + e.getMessage());
        }
    }
}