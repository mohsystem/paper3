import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse107 {

    /**
     * Creates a file, writes content to it, sets restrictive permissions, and prints them.
     * This method uses POSIX file permissions for atomic and secure permission setting.
     *
     * @param fileName The name of the file to create.
     * @param content  The string content to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean createAndSecureFile(String fileName, String content) {
        Path path = Paths.get(fileName);
        
        try {
            // Define restrictive permissions (owner read/write, 0600) for POSIX systems
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);

            // Atomically create the file with specified permissions and write content.
            // Using CREATE_NEW to avoid overwriting existing files.
            try (SeekableByteChannel sbc = Files.newByteChannel(path, 
                    EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE), attr)) {
                sbc.write(ByteBuffer.wrap(content.getBytes(StandardCharsets.UTF_8)));
                System.out.println("Successfully created and wrote to " + fileName);
            }

            // Read and print the file permissions
            Set<PosixFilePermission> actualPerms = Files.getPosixFilePermissions(path);
            System.out.println("File permissions for " + fileName + ": " + PosixFilePermissions.toString(actualPerms));

            return true;
        } catch (UnsupportedOperationException e) {
            // This happens on file systems that do not support POSIX permissions (e.g., default Windows).
            System.err.println("POSIX permissions not supported. Attempting non-atomic fallback.");
            return createAndSecureFileNonPosix(path, content);
        } catch (FileAlreadyExistsException e) {
            System.err.println("Error: File already exists: " + fileName);
            return false;
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        }
    }

    /**
     * Fallback method for non-POSIX systems like Windows.
     * WARNING: This method is not atomic and is vulnerable to TOCTOU (Time-of-check to Time-of-use) attacks.
     */
    private static boolean createAndSecureFileNonPosix(Path path, String content) {
        try {
            Files.writeString(path, content, StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
            System.out.println("Successfully created and wrote to " + path.getFileName());

            java.io.File file = path.toFile();
            // Set permissions for owner only
            if (file.setReadable(false, false) && file.setReadable(true, true) &&
                file.setWritable(false, false) && file.setWritable(true, true) &&
                file.setExecutable(false, false)) {
                System.out.println("File permissions set to owner-only read/write.");
            } else {
                System.err.println("Warning: Could not set all file permissions.");
            }

            System.out.println("Permissions check:");
            System.out.println("  Readable: " + file.canRead());
            System.out.println("  Writable: " + file.canWrite());
            System.out.println("  Executable: " + file.canExecute());
            
            return true;
        } catch (IOException e) {
            System.err.println("An I/O error occurred during non-POSIX fallback: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        String fileName = "secret.txt";
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            boolean success = createAndSecureFile(fileName, content);
            if (success) {
                System.out.println("Test case " + i + " successful.");
            } else {
                System.err.println("Test case " + i + " failed.");
            }

            // Cleanup for the next test case
            try {
                Files.deleteIfExists(Paths.get(fileName));
            } catch (IOException e) {
                System.err.println("Failed to clean up file: " + e.getMessage());
            }
            System.out.println();
        }
    }
}