import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse100 {

    /**
     * Creates a file with secure permissions (owner read/write only).
     * This uses POSIX attributes for an atomic and secure creation.
     * Includes a basic fallback for non-POSIX filesystems like on Windows.
     *
     * @param filePath The path of the file to create.
     * @return true if the file was created successfully, false otherwise.
     */
    public static boolean createSecureFile(String filePath) {
        Path path = Paths.get(filePath);

        // This check is for POSIX-compliant systems (like Linux, macOS)
        if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
            // Permissions: rw------- (owner read/write, no access for group/others)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
            try {
                // Files.createFile is an atomic operation.
                Files.createFile(path, attr);
                System.out.println("Successfully created secure POSIX file: " + filePath);
                return true;
            } catch (FileAlreadyExistsException e) {
                System.err.println("Error: File already exists: " + e.getFile());
                return false;
            } catch (IOException e) {
                // Catches other I/O errors, like permission denied for parent directory
                System.err.println("An I/O error occurred: " + e.getMessage());
                return false;
            }
        } else {
            // Fallback for non-POSIX systems (e.g., Windows)
            System.out.println("POSIX attributes not supported. Using fallback for: " + filePath);
            try {
                // createNewFile is atomic, but setting permissions afterwards is not.
                // This is a best-effort for non-POSIX systems.
                if (path.toFile().createNewFile()) {
                    path.toFile().setReadable(false, false); // Deny for everyone
                    path.toFile().setWritable(false, false); // Deny for everyone
                    path.toFile().setReadable(true, true);  // Allow for owner only
                    path.toFile().setWritable(true, true);  // Allow for owner only
                    System.out.println("Successfully created file with fallback permissions.");
                    return true;
                } else {
                    System.err.println("Error: File already exists (fallback): " + filePath);
                    return false;
                }
            } catch (IOException e) {
                System.err.println("Error creating file (fallback): " + e.getMessage());
                return false;
            }
        }
    }

    public static void main(String[] args) {
        String fileName1 = "important_config";
        String fileName2 = "another_config";
        String invalidPath = "non_existent_dir/important_config";
        String relativePath = "./temp_config";

        System.out.println("--- Test Case 1: Create new file ---");
        createSecureFile(fileName1);

        System.out.println("\n--- Test Case 2: Attempt to create existing file ---");
        createSecureFile(fileName1);

        System.out.println("\n--- Test Case 3: Create a different new file ---");
        createSecureFile(fileName2);

        System.out.println("\n--- Test Case 4: Attempt to create file in non-existent directory ---");
        createSecureFile(invalidPath);
        
        System.out.println("\n--- Test Case 5: Create a file with a relative path ---");
        createSecureFile(relativePath);

        // Cleanup
        System.out.println("\n--- Cleaning up created files ---");
        try {
            Files.deleteIfExists(Paths.get(fileName1));
            Files.deleteIfExists(Paths.get(fileName2));
            Files.deleteIfExists(Paths.get(relativePath));
            System.out.println("Cleanup complete.");
        } catch (IOException e) {
            System.err.println("Error during cleanup: " + e.getMessage());
        }
    }
}