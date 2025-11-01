import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;
import java.util.EnumSet;

public class Lse100 {

    /**
     * Creates a file with secure permissions (owner read/write only).
     * This method is designed for POSIX-compliant file systems.
     * On non-POSIX systems (like Windows), it will create the file with default permissions.
     *
     * @param filename The name of the file to create.
     * @return true if the file was created successfully, false otherwise.
     */
    public static boolean createSecureFile(String filename) {
        Path path = Paths.get(filename);
        
        // Define permissions 0600 (owner read/write)
        Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);

        try {
            // Atomically create the file with the specified permissions.
            // This will fail if the file already exists.
            // On non-POSIX systems, the attribute is ignored.
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                 Files.createFile(path);
            } else {
                 Files.createFile(path, attr);
            }
            System.out.println("Successfully created file: " + filename);
            return true;
        } catch (java.nio.file.FileAlreadyExistsException e) {
            System.err.println("Error creating file: " + filename + " already exists.");
            return false;
        } catch (UnsupportedOperationException e) {
            // This can happen on file systems that do not support POSIX permissions.
            // We can fall back to creating the file with default permissions.
            System.out.println("Warning: POSIX permissions not supported. Creating file with default permissions.");
            try {
                Files.createFile(path);
                System.out.println("Successfully created file with default permissions: " + filename);
                // Note: On a real system, you might want to attempt to set permissions differently
                // for Windows here, but the atomic creation is the main security goal.
                return true;
            } catch (IOException ioException) {
                System.err.println("Error creating file with default permissions: " + ioException.getMessage());
                return false;
            }
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testFiles = {
            "important_config_1.txt",
            "important_config_2.cfg",
            "another_secure_file.dat",
            "test_config.ini",
            "user_settings.json"
        };
        
        System.out.println("--- Running 5 Test Cases ---");

        for (int i = 0; i < testFiles.length; i++) {
            String filename = testFiles[i];
            System.out.println("\n--- Test Case " + (i + 1) + ": " + filename + " ---");
            
            // 1. Attempt to create the file (should succeed)
            System.out.println("Attempt 1: Creating file.");
            createSecureFile(filename);

            // 2. Attempt to create the same file again (should fail)
            System.out.println("Attempt 2: Creating the same file again.");
            createSecureFile(filename);

            // 3. Clean up the created file
            try {
                Files.deleteIfExists(Paths.get(filename));
                System.out.println("Cleaned up file: " + filename);
            } catch (IOException e) {
                System.err.println("Failed to clean up file: " + filename);
            }
        }
    }
}