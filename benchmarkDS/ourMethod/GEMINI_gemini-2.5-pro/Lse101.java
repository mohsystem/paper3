import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Lse101 {

    /**
     * Creates a file with the given name and writes the specified content to it securely.
     * This method uses a temporary file and an atomic move operation to prevent race conditions
     * and ensure the file is written completely or not at all.
     *
     * @param fileName The name of the file to create.
     * @param content  The content to write to the file.
     * @return true if the file was written successfully, false otherwise.
     */
    public static boolean writeImportantConfig(String fileName, String content) {
        Path targetFile = Paths.get(fileName).toAbsolutePath();
        Path parentDir = targetFile.getParent();
        if (parentDir == null) {
            // Should not happen with toAbsolutePath, but as a fallback.
            parentDir = Paths.get(".");
        }

        Path tempFile = null;
        try {
            // Create a temporary file in the same directory as the target to ensure
            // the atomic move operation is possible across most filesystems.
            // createTempFile uses secure default permissions (typically 0600 on POSIX).
            tempFile = Files.createTempFile(parentDir, "config", ".tmp");

            // Write content to the temporary file.
            Files.write(tempFile, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.WRITE);
            
            // Atomically move the temporary file to the final destination.
            // REPLACE_EXISTING ensures that if the target file exists, it's replaced.
            Files.move(tempFile, targetFile, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);

            // fsync the directory to ensure the rename is persisted to the filesystem.
            try (FileChannel channel = FileChannel.open(parentDir, StandardOpenOption.READ)) {
                channel.force(true);
            }

            return true;
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
            // Clean up the temporary file if the operation failed.
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException cleanupEx) {
                    System.err.println("Error cleaning up temporary file: " + cleanupEx.getMessage());
                }
            }
            return false;
        }
    }

    public static void main(String[] args) {
        final String fileName = "important_config";
        final String content = "important_config";

        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            boolean success = writeImportantConfig(fileName, content);
            if (success) {
                System.out.println("Successfully wrote to " + fileName);
                // Optional: verify content
                try {
                    String readContent = new String(Files.readAllBytes(Paths.get(fileName)), StandardCharsets.UTF_8);
                    System.out.println("File content verification: " + (content.equals(readContent) ? "PASSED" : "FAILED"));
                } catch (IOException e) {
                     System.out.println("Failed to read back file for verification.");
                }
            } else {
                System.out.println("Failed to write to " + fileName);
            }
            System.out.println("--------------------");
        }
    }
}