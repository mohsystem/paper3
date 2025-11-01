import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse106 {

    /**
     * Creates a file, writes secret information to it, and changes its permissions
     * to be read-only for the owner.
     *
     * @param secretInfo The string content to write to the file.
     * @return The absolute path of the created file, or null on failure.
     */
    public static String createSecretFile(String secretInfo) {
        Path filePath = null;
        try {
            // Create a temporary file to avoid race conditions and hardcoded names.
            // This is a security best practice.
            filePath = Files.createTempFile("secret_", ".txt");

            // Write the super secret info to the file.
            Files.writeString(filePath, secretInfo, StandardOpenOption.WRITE);

            // Change the permissions of the file to read-only for the owner (0400).
            // This is the POSIX-specific way.
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(filePath, perms);
            } catch (UnsupportedOperationException e) {
                // Fallback for non-POSIX systems (like Windows).
                System.err.println("Warning: POSIX permissions not supported. Using basic file permissions as a fallback.");
                java.io.File file = filePath.toFile();
                file.setReadable(true, true);  // Owner read only
                file.setWritable(false, true); // Owner cannot write
                file.setExecutable(false, true); // Owner cannot execute
            }

            // Return the absolute path of the file.
            return filePath.toAbsolutePath().toString();

        } catch (IOException e) {
            System.err.println("An error occurred: " + e.getMessage());
            // Clean up the file if it was created but an error occurred later.
            if (filePath != null) {
                try {
                    Files.deleteIfExists(filePath);
                } catch (IOException cleanupEx) {
                    System.err.println("Failed to clean up file: " + cleanupEx.getMessage());
                }
            }
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running Java Test Cases:");
        for (int i = 1; i <= 5; i++) {
            String secret = "Java super secret key #" + i + " is JVK" + (i * 123);
            String fileName = createSecretFile(secret);
            if (fileName != null) {
                System.out.println("Test Case " + i + ": Secret file created at: " + fileName);
                // In a real application, you would manage the lifecycle of this file.
                // For this example, we can ask the JVM to delete it on exit.
                new java.io.File(fileName).deleteOnExit();
            } else {
                System.out.println("Test Case " + i + ": Failed to create secret file.");
            }
        }
    }
}