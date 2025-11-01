import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse105 {

    /**
     * Creates a file with the given content and restrictive permissions.
     * On POSIX systems, permissions are set to 600 (owner read/write).
     *
     * @param filename The name of the file to create (e.g., "secret.txt").
     * @param content  The string content to write to the file.
     * @return true if the file was created and secured successfully, false otherwise.
     */
    public static boolean createSecretFile(String filename, String content) {
        Path path = Paths.get(filename);

        // This primary implementation is POSIX-specific for maximum security (atomic permission setting).
        // A fallback for non-POSIX systems like Windows is included.
        try {
            // Define restrictive permissions: owner can read and write (rw------- or 600)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
            
            // Delete the file if it exists to ensure a clean start for the test.
            Files.deleteIfExists(path);

            // Create the file with the specified permissions atomically.
            Files.createFile(path, attr);
            
            // Write content to the newly created file.
            Files.writeString(path, content, StandardOpenOption.WRITE);
            
            System.out.println("Successfully created and secured '" + filename + "'.");
            return true;
        } catch (UnsupportedOperationException e) {
            // This exception is thrown on systems that do not support POSIX file attributes (like Windows).
            System.err.println("POSIX permissions not supported. Trying a less secure fallback for Windows.");
            try {
                // On non-POSIX systems, we create the file first, then set permissions.
                // This introduces a small window where the file exists with default permissions.
                Files.writeString(path, content, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
                
                // Attempt to set owner-only read/write permissions.
                path.toFile().setReadable(false, false); // clear all read permissions
                path.toFile().setWritable(false, false); // clear all write permissions
                path.toFile().setReadable(true, true);   // set owner read only
                path.toFile().setWritable(true, true);   // set owner write only
                System.out.println("Successfully created '" + filename + "' with basic restricted permissions.");
                return true;
            } catch (IOException ioe) {
                System.err.println("Error during fallback file creation: " + ioe.getMessage());
                return false;
            }
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String filename = "secret.txt";
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            
            // The createSecretFile function handles cleanup by deleting the file if it exists.
            boolean success = createSecretFile(filename, content);
            
            System.out.println("Test " + i + " result: " + (success ? "Success" : "Failure"));
            System.out.println();
        }
    }
}