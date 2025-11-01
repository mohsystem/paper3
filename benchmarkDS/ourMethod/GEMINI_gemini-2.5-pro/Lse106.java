import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class Lse106 {

    /**
     * Creates a file, writes secret information to it, and sets its permissions to read-only for the owner.
     *
     * @param secretInfo The secret information to write to the file.
     * @return The absolute path of the created file, or null on failure.
     */
    public static String createSecretFile(String secretInfo) {
        Path tempFile = null;
        try {
            // Create a temporary file securely.
            // On POSIX systems, this typically uses 0600 permissions by default.
            tempFile = Files.createTempFile("secret-", ".txt");
            File file = tempFile.toFile();

            // Best effort to ensure the file is cleaned up on JVM exit.
            file.deleteOnExit();

            // Write the secret information to the file using UTF-8 encoding.
            Files.write(tempFile, secretInfo.getBytes(java.nio.charset.StandardCharsets.UTF_8));

            // Change permissions to be read-only for the owner (equivalent to 0400 on POSIX).
            // This is a cross-platform approach.
            // First, revoke all permissions for everyone.
            boolean success = file.setReadable(false, false) &&
                              file.setWritable(false, false) &&
                              file.setExecutable(false, false);
            
            if (!success) {
                 System.err.println("Warning: Could not clear all permissions initially.");
            }

            // Then, grant read permission only to the owner.
            if (!file.setReadable(true, true)) {
                // If this fails, the file might be left in an insecure state.
                Files.delete(tempFile);
                throw new IOException("Failed to set file permissions to read-only for owner.");
            }

            // Return the absolute path of the file.
            return file.getAbsolutePath();

        } catch (IOException e) {
            System.err.println("An error occurred: " + e.getMessage());
            // Clean up the file if it was partially created.
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException cleanupEx) {
                    System.err.println("Failed to cleanup temporary file: " + cleanupEx.getMessage());
                }
            }
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running Java test cases...");
        // This list is kept for demonstration, but files are deleted on exit.
        List<String> createdFiles = new ArrayList<>();

        for (int i = 0; i < 5; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            String secret = "Java super secret info #" + (i + 1);
            String fileName = createSecretFile(secret);

            if (fileName != null) {
                System.out.println("Secret file created: " + fileName);
                createdFiles.add(fileName);
                
                // Verify file properties
                File f = new File(fileName);
                System.out.println("File exists: " + f.exists());
                System.out.println("Owner can read: " + f.canRead());
                System.out.println("Owner can write: " + f.canWrite());
            } else {
                System.err.println("Failed to create secret file.");
            }
        }
        
        System.out.println("\nJava tests finished. Files scheduled for deletion on JVM exit.");
    }
}