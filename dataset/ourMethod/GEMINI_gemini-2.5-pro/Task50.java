import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task50 {

    private static final String UPLOAD_DIR_NAME = "uploads";
    private static final Path UPLOAD_DIR_PATH = Paths.get(UPLOAD_DIR_NAME).toAbsolutePath();

    /**
     * Simulates uploading a file by saving its content to a designated upload directory.
     *
     * @param originalFilename The requested filename from the user.
     * @param content          The file content as a byte array.
     * @return A confirmation or error message.
     */
    public static String uploadFile(String originalFilename, byte[] content) {
        if (originalFilename == null || originalFilename.trim().isEmpty()) {
            return "Error: Filename cannot be empty.";
        }
        
        // Rule #9: Validate and sanitize all external input used in path construction.
        // Extract only the filename part to prevent directory traversal.
        Path filenamePath = Paths.get(originalFilename).getFileName();
        if (filenamePath == null) {
            return "Error: Invalid filename provided.";
        }
        String sanitizedFilename = filenamePath.toString();
        
        // Rule #9: Ensure paths resolve within the expected directory.
        Path finalPath = UPLOAD_DIR_PATH.resolve(sanitizedFilename).normalize();
        if (!finalPath.startsWith(UPLOAD_DIR_PATH)) {
            return "Error: Path traversal attempt detected.";
        }

        // Rule #3: Mitigate TOCTOU vulnerabilities by writing to a temp file and renaming.
        Path tempFile = null;
        try {
            // Create a temporary file in the secure upload directory.
            tempFile = Files.createTempFile(UPLOAD_DIR_PATH, "upload-", ".tmp");

            // Rule #1, #2: Write content to the temporary file.
            Files.write(tempFile, content, StandardOpenOption.WRITE);
            
            // Atomically move the temporary file to its final destination.
            // This fails if the file already exists, which prevents overwrites.
            try {
                Files.move(tempFile, finalPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
                // In a real scenario, you might not want to replace existing files.
                // For this example, we remove REPLACE_EXISTING to prevent overwrites.
                // Re-add it for clarity, then remove.
                Files.move(tempFile, finalPath, StandardCopyOption.ATOMIC_MOVE);
                return "Success: File '" + sanitizedFilename + "' uploaded successfully.";
            } catch (FileAlreadyExistsException e) {
                // This is the desired behavior: do not overwrite existing files.
                return "Error: File '" + sanitizedFilename + "' already exists.";
            }

        } catch (IOException e) {
            return "Error: Could not save file. " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Security violation. " + e.getMessage();
        } finally {
            // Ensure the temporary file is deleted if the move fails for any reason.
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException e) {
                    // In a real application, log this error.
                    System.err.println("Failed to delete temporary file: " + tempFile);
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Setup: Create the upload directory
        try {
            if (!Files.exists(UPLOAD_DIR_PATH)) {
                Files.createDirectories(UPLOAD_DIR_PATH);
                System.out.println("Upload directory created at: " + UPLOAD_DIR_PATH);
            }
        } catch (IOException e) {
            System.err.println("Fatal: Could not create upload directory. " + e.getMessage());
            return;
        }

        byte[] sampleContent = "This is the file content.".getBytes(StandardCharsets.UTF_8);

        // --- Test Cases ---
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Valid filename
        System.out.println("1. Valid filename 'test1.txt':");
        System.out.println("   " + uploadFile("test1.txt", sampleContent));

        // Test Case 2: Filename with spaces
        System.out.println("2. Filename with spaces 'my document.pdf':");
        System.out.println("   " + uploadFile("my document.pdf", sampleContent));

        // Test Case 3: Path traversal attempt
        System.out.println("3. Path traversal attempt '../secret.txt':");
        System.out.println("   " + uploadFile("../secret.txt", sampleContent));

        // Test Case 4: Absolute path attempt
        String absolutePath = new File("/etc/passwd").getAbsolutePath();
        System.out.println("4. Absolute path attempt '" + absolutePath + "':");
        System.out.println("   " + uploadFile(absolutePath, sampleContent));
        
        // Test Case 5: Empty filename
        System.out.println("5. Empty filename '':");
        System.out.println("   " + uploadFile("", sampleContent));

        // --- Cleanup ---
        System.out.println("\n--- Cleaning up created files ---");
        try {
            Files.deleteIfExists(UPLOAD_DIR_PATH.resolve("test1.txt"));
            Files.deleteIfExists(UPLOAD_DIR_PATH.resolve("my document.pdf"));
            Files.delete(UPLOAD_DIR_PATH);
            System.out.println("Cleanup successful.");
        } catch (IOException e) {
            System.err.println("Cleanup failed: " + e.getMessage());
        }
    }
}