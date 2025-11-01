import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse100 {

    /**
     * Creates a file with secure permissions (owner read/write only on POSIX).
     * The creation is atomic; it fails if the file already exists.
     *
     * @param filePath The path to the file to be created.
     * @return true if the file was created successfully, false otherwise.
     */
    public static boolean createSecureFile(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty.");
            return false;
        }

        Path path = Paths.get(filePath);

        // Security: Prevent path traversal attacks.
        // A normalized path of a legitimate file within the current working directory
        // should not contain "..".
        if (path.normalize().toString().contains("..")) {
             System.err.println("Error: Invalid file path (contains '..').");
             return false;
        }

        // Check if the parent directory exists before attempting to create the file.
        Path parentDir = path.getParent();
        if (parentDir != null && !Files.exists(parentDir)) {
             System.err.println("Error: Parent directory does not exist for path: " + filePath);
             return false;
        }

        try {
            String os = System.getProperty("os.name").toLowerCase();
            // On POSIX-compliant systems (Linux, macOS), set specific permissions.
            if (!os.contains("win")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                Files.createFile(path, attr);
            } else {
                // On Windows, default permissions are generally secure for the creating user.
                // We must check for existence first to mimic atomic O_EXCL behavior.
                if (Files.exists(path)) {
                    throw new FileAlreadyExistsException(filePath);
                }
                Files.createFile(path);
            }
            System.out.println("Successfully created secure file: " + filePath);
            return true;
        } catch (FileAlreadyExistsException e) {
            System.err.println("Error: File already exists: " + filePath);
            return false;
        } catch (UnsupportedOperationException e) {
            // This occurs if the file system does not support POSIX permissions.
            // We can fall back to creating the file with default permissions.
            System.err.println("Warning: POSIX permissions not supported. Creating with default permissions.");
            try {
                 if (Files.exists(path)) {
                    System.err.println("Error: File already exists: " + filePath);
                    return false;
                }
                Files.createFile(path);
                System.out.println("Successfully created file with default permissions: " + filePath);
                return true;
            } catch (IOException ioe) {
                 System.err.println("Error creating file with default permissions: " + ioe.getMessage());
                 return false;
            }
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Security error: Do not have permission to create the file: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Standard successful creation
        System.out.println("\nTest Case 1: Create 'important_config.java'");
        createSecureFile("important_config.java");

        // Test Case 2: Attempt to create a file that already exists
        System.out.println("\nTest Case 2: Attempt to create existing file 'important_config.java'");
        createSecureFile("important_config.java");

        // Test Case 3: Create a file with a relative path
        System.out.println("\nTest Case 3: Create './another_config.java.cfg'");
        createSecureFile("./another_config.java.cfg");

        // Test Case 4: Attempt to create a file in a non-existent directory
        System.out.println("\nTest Case 4: Attempt to create in a non-existent directory");
        createSecureFile("non_existent_dir/some_file.txt");

        // Test Case 5: Another successful creation
        System.out.println("\nTest Case 5: Create 'config_3.dat'");
        createSecureFile("config_3.dat");
    }
}