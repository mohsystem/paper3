import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;

public class Lse48 {

    /**
     * Securely reads a file from a base directory, preventing path traversal.
     *
     * @param baseDirectory The secure directory from which files can be served.
     * @param fileName      The name of the file requested by the user.
     * @return A byte array of the file contents, or null if the file is not found,
     *         not accessible, or the request is insecure.
     */
    public static byte[] img(String baseDirectory, String fileName) {
        // Create Path objects for the base directory and the requested file.
        Path baseDir = Paths.get(baseDirectory).toAbsolutePath();
        Path requestedFile = baseDir.resolve(fileName).toAbsolutePath();

        // *** SECURITY CHECK ***
        // To prevent path traversal attacks (e.g., ../../etc/passwd), we ensure the
        // resolved path of the requested file is still within the base directory.
        if (!requestedFile.normalize().startsWith(baseDir)) {
            System.err.println("Security alert: Path traversal attempt detected for file: " + fileName);
            return null;
        }

        // Check if the file exists and is a regular file.
        if (!Files.exists(requestedFile) || !Files.isRegularFile(requestedFile)) {
             // System.err.println("Error: File not found or is not a regular file: " + requestedFile);
            return null;
        }

        try {
            return Files.readAllBytes(requestedFile);
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        }
    }

    // Helper method to run and display test results
    private static void runTest(String testName, String baseDir, String fileName) {
        System.out.println("--- Running Test: " + testName + " ---");
        System.out.println("Requesting file: " + fileName);
        byte[] content = img(baseDir, fileName);
        if (content != null) {
            System.out.println("Result: SUCCESS - Read " + content.length + " bytes.");
            // System.out.println("Content: " + new String(content));
        } else {
            System.out.println("Result: FAILED (or file securely blocked) - as expected for invalid cases.");
        }
        System.out.println();
    }
    
    // Main method for testing
    public static void main(String[] args) {
        // Setup temporary directory and files for testing
        String baseDir = "safe_image_dir";
        String unsafeDir = "unsafe_dir";
        File baseDirFile = new File(baseDir);
        File unsafeDirFile = new File(unsafeDir);
        
        try {
            // Create directories
            baseDirFile.mkdir();
            new File(baseDir + File.separator + "subdir").mkdir();
            unsafeDirFile.mkdir();

            // Create files
            try (PrintWriter writer = new PrintWriter(baseDir + "/image1.jpg", "UTF-8")) {
                writer.println("jpeg_data_for_image1");
            }
            try (PrintWriter writer = new PrintWriter(baseDir + "/subdir/image2.png", "UTF-8")) {
                writer.println("png_data_for_image2");
            }
            try (PrintWriter writer = new PrintWriter(unsafeDir + "/secret.txt", "UTF-8")) {
                writer.println("this is a secret file");
            }

            // --- Test Cases ---
            // 1. Valid file in base directory
            runTest("Valid File", baseDir, "image1.jpg");

            // 2. Valid file in subdirectory
            runTest("Valid Subdirectory File", baseDir, "subdir/image2.png");
            
            // 3. Non-existent file
            runTest("Non-Existent File", baseDir, "notfound.gif");

            // 4. Path traversal attack
            // On Windows, use "..\\unsafe_dir\\secret.txt"
            String traversalPath = ".." + File.separator + unsafeDir + File.separator + "secret.txt";
            runTest("Path Traversal Attack", baseDir, traversalPath);
            
            // 5. Absolute path attack (will fail because resolve treats it as relative)
            // A more direct absolute path would be handled by the startsWith check.
            File secretFile = new File(unsafeDir + "/secret.txt");
            runTest("Absolute Path Attack", baseDir, secretFile.getAbsolutePath());

        } catch (IOException e) {
            System.err.println("Setup failed: " + e.getMessage());
        } finally {
            // Cleanup
            new File(baseDir + "/image1.jpg").delete();
            new File(baseDir + "/subdir/image2.png").delete();
            new File(baseDir + "/subdir").delete();
            baseDirFile.delete();
            new File(unsafeDir + "/secret.txt").delete();
            unsafeDirFile.delete();
        }
    }
}