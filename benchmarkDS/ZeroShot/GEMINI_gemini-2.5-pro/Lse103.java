import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.List;

public class Lse103 {

    /**
     * Opens or creates a file with the specified options, demonstrating file flag behaviors.
     * This function is secure because it uses try-with-resources to ensure the file stream
     * is always closed, preventing resource leaks. It also relies on the robust and secure
     * java.nio.file package.
     *
     * @param fileName The name of the file to open.
     * @param options  The StandardOpenOption flags to use (e.g., CREATE, WRITE, TRUNCATE_EXISTING).
     * @return true if the file was opened successfully, false otherwise.
     */
    public static boolean openFile(String fileName, StandardOpenOption... options) {
        Path path = Paths.get(fileName);
        try {
            // The try-with-resources statement ensures the stream is closed automatically.
            // We don't need to do anything with the stream; successfully opening it is the goal.
            try (OutputStream os = Files.newOutputStream(path, options)) {
                // File opened successfully
            }
            return true;
        } catch (IOException e) {
            // Catches exceptions like FileAlreadyExistsException, NoSuchFileException, etc.
            System.err.println("Operation failed for " + fileName + ": " + e.toString());
            return false;
        }
    }

    // Helper function for test setup/cleanup
    private static void setupFile(String filename, String content) {
        try {
            Path path = Paths.get(filename);
            if (content == null) {
                Files.deleteIfExists(path);
            } else {
                Files.write(path, content.getBytes());
            }
        } catch (IOException e) {
            System.err.println("Test setup failed for " + filename + ": " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        List<String> testFiles = Arrays.asList("test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt");

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Create a new file exclusively (O_CREAT | O_WRONLY | O_EXCL)
        // Should succeed as the file does not exist.
        System.out.println("\n[Test 1] Create new file exclusively (CREATE_NEW)");
        String file1 = "test1.txt";
        setupFile(file1, null); // Ensure file does not exist
        boolean result1 = openFile(file1, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        System.out.println("Result: " + (result1 ? "Success" : "Failure") + " (Expected: Success)");

        // Test Case 2: Attempt to create an existing file exclusively (O_CREAT | O_WRONLY | O_EXCL)
        // Should fail as the file already exists.
        System.out.println("\n[Test 2] Attempt exclusive creation on existing file (CREATE_NEW)");
        String file2 = "test2.txt";
        setupFile(file2, "content"); // Ensure file exists
        boolean result2 = openFile(file2, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        System.out.println("Result: " + (result2 ? "Success" : "Failure") + " (Expected: Failure)");

        // Test Case 3: Open an existing file and truncate it (O_WRONLY | O_TRUNC)
        // Should succeed and the file content will be cleared.
        System.out.println("\n[Test 3] Open and truncate existing file (TRUNCATE_EXISTING)");
        String file3 = "test3.txt";
        setupFile(file3, "some initial content"); // Ensure file exists with content
        boolean result3 = openFile(file3, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
        System.out.println("Result: " + (result3 ? "Success" : "Failure") + " (Expected: Success)");
        
        // Test Case 4: Open an existing file for writing only (O_WRONLY)
        // Should succeed without truncating the file upon opening.
        System.out.println("\n[Test 4] Open existing file for writing (WRITE)");
        String file4 = "test4.txt";
        setupFile(file4, "content"); // Ensure file exists
        boolean result4 = openFile(file4, StandardOpenOption.WRITE);
        System.out.println("Result: " + (result4 ? "Success" : "Failure") + " (Expected: Success)");

        // Test Case 5: Create a file if it does not exist (O_CREAT | O_WRONLY)
        // Should succeed.
        System.out.println("\n[Test 5] Create file if it doesn't exist (CREATE)");
        String file5 = "test5.txt";
        setupFile(file5, null); // Ensure file does not exist
        boolean result5 = openFile(file5, StandardOpenOption.CREATE, StandardOpenOption.WRITE);
        System.out.println("Result: " + (result5 ? "Success" : "Failure") + " (Expected: Success)");

        // Cleanup
        System.out.println("\nCleaning up test files...");
        for(String file : testFiles) {
            setupFile(file, null);
        }
        System.out.println("Done.");
    }
}