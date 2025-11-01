import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.stream.Collectors;
import java.io.BufferedReader;
import java.io.FileReader;

public class Lse44 {

    private static final String SAFE_DIRECTORY = "/safe/";

    /**
     * Reads a file from the predefined SAFE_DIRECTORY, preventing path traversal.
     *
     * @param filename The name of the file to read.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFile(String filename) {
        if (filename == null || filename.isEmpty() || filename.contains("..")) {
            return "Error: Invalid filename provided.";
        }

        Path safeDirPath = Paths.get(SAFE_DIRECTORY).toAbsolutePath();
        Path requestedFilePath = safeDirPath.resolve(filename).normalize();

        try {
            // Security Check: Ensure the resolved path is still within the safe directory.
            if (!requestedFilePath.toRealPath().startsWith(safeDirPath.toRealPath())) {
                return "Error: Directory traversal attempt detected.";
            }

            if (Files.isReadable(requestedFilePath) && !Files.isDirectory(requestedFilePath)) {
                 try (BufferedReader reader = new BufferedReader(new FileReader(requestedFilePath.toFile()))) {
                    return reader.lines().collect(Collectors.joining(System.lineSeparator()));
                }
            } else {
                return "Error: File is not readable or does not exist.";
            }

        } catch (IOException e) {
            // Catches errors like file not found, permission denied during realpath resolution
            return "Error: Could not read the file. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Setup test environment
        try {
            Path safeDir = Paths.get(SAFE_DIRECTORY);
            if (!Files.exists(safeDir)) {
                Files.createDirectories(safeDir);
            }
            Files.write(safeDir.resolve("test1.txt"), "This is a safe file.".getBytes());
            Files.write(safeDir.resolve("test2.log"), "Another safe log file.".getBytes());
            
            // Create a file outside the safe directory for traversal test
            Files.write(Paths.get("unsafe.txt"), "This is an unsafe file.".getBytes());

        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Valid file
        System.out.println("1. Reading 'test1.txt':");
        System.out.println(readFile("test1.txt"));
        System.out.println();

        // Test Case 2: Another valid file
        System.out.println("2. Reading 'test2.log':");
        System.out.println(readFile("test2.log"));
        System.out.println();

        // Test Case 3: Non-existent file
        System.out.println("3. Reading 'nonexistent.txt':");
        System.out.println(readFile("nonexistent.txt"));
        System.out.println();

        // Test Case 4: Path traversal attempt
        System.out.println("4. Reading '../unsafe.txt':");
        System.out.println(readFile("../unsafe.txt"));
        System.out.println();
        
        // Test Case 5: Absolute path attempt
        System.out.println("5. Reading '/etc/passwd':");
        System.out.println(readFile("/etc/passwd"));
        System.out.println();
    }
}