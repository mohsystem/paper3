import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse105 {

    /**
     * Creates a file with the given name and content, setting restrictive permissions.
     * On POSIX-compliant systems, this is an atomic operation.
     *
     * @param fileName The name of the file to create.
     * @param content  The string content to write to the file.
     * @return true if the file was created successfully, false otherwise.
     */
    public static boolean createSecretFile(String fileName, String content) {
        Path filePath = Paths.get(fileName);
        byte[] contentBytes = content.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.wrap(contentBytes);

        // Define restrictive POSIX permissions (owner read/write, 0600)
        Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
        
        // Use a try-with-resources statement to ensure the channel is closed.
        // StandardOpenOption.CREATE_NEW: Fails if the file already exists.
        // StandardOpenOption.WRITE: Opens the file for writing.
        try (FileChannel fc = FileChannel.open(filePath, EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE), attr)) {
            fc.write(buffer);
            System.out.println("Successfully created and secured file: " + fileName);
            return true;
        } catch (UnsupportedOperationException e) {
            // This happens on non-POSIX systems like Windows where the attribute is not supported.
            // Fallback to a less secure, non-atomic method.
            System.out.println("Warning: POSIX permissions not supported. Attempting fallback for " + fileName);
            try {
                Files.writeString(filePath, content, StandardOpenOption.CREATE_NEW);
                // Set permissions after creation (best effort for non-POSIX)
                if (!filePath.toFile().setReadable(false, false) ||
                    !filePath.toFile().setWritable(false, false) ||
                    !filePath.toFile().setExecutable(false, false) ||
                    !filePath.toFile().setReadable(true, true) ||
                    !filePath.toFile().setWritable(true, true)) {
                    System.out.println("Warning: Could not set all restrictive permissions for " + fileName);
                }
                System.out.println("Successfully created file with fallback permissions: " + fileName);
                return true;
            } catch (java.nio.file.FileAlreadyExistsException faee) {
                 System.err.println("Error: File already exists: " + fileName);
                 return false;
            } catch (IOException ioex) {
                 System.err.println("An I/O error occurred during fallback for " + fileName + ": " + ioex.getMessage());
                 return false;
            }
        } catch (java.nio.file.FileAlreadyExistsException e) {
            System.err.println("Error: File already exists: " + fileName);
            return false;
        } catch (IOException e) {
            System.err.println("An I/O error occurred for " + fileName + ": " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        final String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

        System.out.println("--- Running Test Cases ---");

        String[] testFiles = {"secret1.txt", "secret2.txt", "secret3.txt", "secret4.txt", "secret5.txt"};

        // Test Cases 1-5: Create files
        for (String fileName : testFiles) {
            System.out.println("\n--- Test: Create " + fileName + " ---");
            createSecretFile(fileName, content);
        }

        // Additional Test: Try creating an existing file to test failure
        System.out.println("\n--- Test: Attempt to re-create secret2.txt (should fail) ---");
        createSecretFile("secret2.txt", content);

        System.out.println("\n--- Cleaning up created files ---");
        for (String fileName : testFiles) {
            try {
                Files.deleteIfExists(Paths.get(fileName));
            } catch (IOException e) {
                System.err.println("Failed to clean up " + fileName + ": " + e.getMessage());
            }
        }
        System.out.println("Cleanup complete.");
    }
}