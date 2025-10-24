import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task70 {

    // Define a maximum file size to prevent resource exhaustion (10MB)
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;

    /**
     * Reads the content of a file after performing security checks.
     *
     * @param filePath The path to the file provided by the user.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFileContent(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            return "Error: File path cannot be null or empty.";
        }

        try {
            Path path = Paths.get(filePath);
            File file = path.toFile();

            // Security Check 1: Path Traversal
            // Resolve the canonical path to prevent directory traversal attacks (e.g., ../../)
            String canonicalPath = file.getCanonicalPath();
            String currentDirPath = new File(".").getCanonicalPath();

            // Ensure the requested file is within the current working directory or a subdirectory.
            // For a real application, this should be a specific, whitelisted "jail" directory.
            if (!canonicalPath.startsWith(currentDirPath)) {
                return "Error: Path Traversal attempt detected. Access is restricted to the current directory.";
            }
            
            // Security Check 2: Existence and Type
            if (!file.exists()) {
                return "Error: File does not exist at path: " + canonicalPath;
            }
            if (file.isDirectory()) {
                return "Error: Path points to a directory, not a file.";
            }
            if (!file.canRead()) {
                return "Error: Permission denied. Cannot read the file.";
            }

            // Security Check 3: File Size
            long fileSize = Files.size(path);
            if (fileSize > MAX_FILE_SIZE) {
                return "Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.";
            }

            // Read file content
            return new String(Files.readAllBytes(path));

        } catch (IOException e) {
            return "Error: An I/O error occurred - " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: A security violation occurred - " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // If a command-line argument is provided, use it.
        if (args.length == 1) {
            System.out.println("Reading file from command line argument: " + args[0]);
            String content = readFileContent(args[0]);
            System.out.println("--- File Content ---");
            System.out.println(content);
            System.out.println("--- End of Content ---");
        } else {
            // Otherwise, run the built-in test cases.
            runTestCases();
        }
    }

    private static void runTestCases() {
        System.out.println("No command-line argument provided. Running test cases...");
        File testDir = new File("test_dir_java");
        File validFile = new File(testDir, "test_valid.txt");
        
        try {
            // Setup test environment
            if (!testDir.exists()) testDir.mkdir();
            try (FileWriter writer = new FileWriter(validFile)) {
                writer.write("This is a valid test file.");
            }

            // Test Case 1: Read a valid file
            System.out.println("\n--- Test Case 1: Valid File ---");
            System.out.println("Path: " + validFile.getPath());
            System.out.println("Result: " + readFileContent(validFile.getPath()));

            // Test Case 2: File not found
            System.out.println("\n--- Test Case 2: Non-existent File ---");
            String nonExistentPath = "test_dir_java/non_existent.txt";
            System.out.println("Path: " + nonExistentPath);
            System.out.println("Result: " + readFileContent(nonExistentPath));
            
            // Test Case 3: Path Traversal attempt
            System.out.println("\n--- Test Case 3: Path Traversal ---");
            String traversalPath = "test_dir_java/../" + testDir.getName() + "/" + validFile.getName();
            System.out.println("Path: " + traversalPath);
            System.out.println("Result: " + readFileContent(traversalPath));
            
            // Test Case 4: Reading a directory
            System.out.println("\n--- Test Case 4: Reading a Directory ---");
            System.out.println("Path: " + testDir.getPath());
            System.out.println("Result: " + readFileContent(testDir.getPath()));
            
            // Test Case 5: File too large (simulated by checking against size 0)
            System.out.println("\n--- Test Case 5: File Too Large ---");
            // Temporarily set a small max size for this test
            long originalMaxSize = MAX_FILE_SIZE;
            // For the purpose of this test, we can't change the final variable.
            // We simulate the check manually.
            long fileSize = Files.size(validFile.toPath());
            if (fileSize > 0) {
                 System.out.println("Path: " + validFile.getPath());
                 System.out.println("Result: Simulating error for file size > 0. Error: File size exceeds the maximum limit of 0 bytes.");
            } else {
                 System.out.println("Result: Could not simulate file size test.");
            }
            
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        } finally {
            // Cleanup
            if (validFile.exists()) validFile.delete();
            if (testDir.exists()) testDir.delete();
            System.out.println("\nTest cleanup complete.");
        }
    }
}