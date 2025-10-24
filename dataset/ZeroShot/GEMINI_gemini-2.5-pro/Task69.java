import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task69 {

    /**
     * Securely deletes a file after validating the path.
     * The file must exist, be a regular file, and be located within the current working directory or its subdirectories.
     * This prevents path traversal attacks (e.g., using "../") to delete files outside the intended scope.
     *
     * @param filePath The path to the file to be deleted.
     * @return true if the file was successfully deleted, false otherwise.
     */
    public static boolean deleteFileSecurely(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path is null or empty.");
            return false;
        }

        try {
            File file = new File(filePath);

            // Security Check 1: Canonicalize paths to resolve '..' and symbolic links.
            String canonicalPath = file.getCanonicalPath();
            String canonicalCwd = new File(".").getCanonicalPath();

            // Security Check 2: Ensure the file path is within the current working directory.
            if (!canonicalPath.startsWith(canonicalCwd)) {
                System.err.println("Error: Path traversal attempt detected. Cannot delete files outside the current directory.");
                return false;
            }

            // Validation Check 1: Ensure the path points to an existing file.
            if (!file.exists()) {
                System.err.println("Error: File does not exist: " + filePath);
                return false;
            }

            // Validation Check 2: Ensure it's a file and not a directory.
            if (!file.isFile()) {
                System.err.println("Error: Path does not point to a regular file (it might be a directory): " + filePath);
                return false;
            }

            // Attempt to delete the file.
            if (file.delete()) {
                System.out.println("Successfully deleted file: " + filePath);
                return true;
            } else {
                System.err.println("Error: Failed to delete file. Check permissions: " + filePath);
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error processing path: " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Security manager prevented file deletion: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println("Attempting to delete file from command line argument: " + args[0]);
            deleteFileSecurely(args[0]);
        } else {
            System.out.println("No command line arguments provided. Running test cases...");
            runTestCases();
        }
    }

    private static void runTestCases() {
        System.out.println("\n--- Running Java Test Cases ---");
        Path safeFile = Paths.get("test_safe.txt");
        Path testDir = Paths.get("test_dir");
        Path unsafeFile = Paths.get("..", "test_unsafe.txt");
        
        // Setup test environment
        try {
            Files.createFile(safeFile);
            Files.createDirectory(testDir);
            System.out.println("Test environment created.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // Run tests
        try {
            // Test 1: Success case - delete a safe file
            System.out.println("\n[Test 1] Deleting a safe local file...");
            deleteFileSecurely(safeFile.toString());

            // Test 2: Failure case - file does not exist
            System.out.println("\n[Test 2] Deleting a non-existent file...");
            deleteFileSecurely("nonexistent.txt");

            // Test 3: Failure case - attempting to delete a directory
            System.out.println("\n[Test 3] Deleting a directory...");
            deleteFileSecurely(testDir.toString());

            // Test 4: Failure case - path traversal
            // Note: We don't create the unsafe file to avoid writing outside the CWD.
            // The security check should prevent the attempt regardless.
            System.out.println("\n[Test 4] Attempting path traversal deletion...");
            deleteFileSecurely(unsafeFile.toString());
            
            // Test 5: Failure case - absolute path (will fail unless CWD is root)
            System.out.println("\n[Test 5] Attempting to delete file with an absolute path...");
            // Using a common temporary directory path for the example
            String absolutePath = new File(System.getProperty("java.io.tmpdir"), "abs_test.txt").getAbsolutePath();
            deleteFileSecurely(absolutePath);

        } finally {
            // Cleanup test environment
            try {
                Files.deleteIfExists(safeFile);
                Files.deleteIfExists(testDir);
                System.out.println("\nTest environment cleaned up.");
            } catch (IOException e) {
                System.err.println("Failed to clean up test environment: " + e.getMessage());
            }
        }
    }
}