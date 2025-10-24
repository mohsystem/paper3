import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;
import java.nio.file.FileSystems;

public class Task101 {

    /**
     * Creates a file and makes it executable.
     * On POSIX systems, it sets permissions to rwxr-xr-x.
     * On other systems (like Windows), it attempts to set the executable flag for all users.
     *
     * @param filename The name of the file to create.
     * @return true if successful, false otherwise.
     */
    public static boolean createAndMakeExecutable(String filename) {
        Path path = Paths.get(filename);

        // For test repeatability, delete the file if it already exists.
        try {
            Files.deleteIfExists(path);
        } catch (IOException e) {
            System.err.println("Error deleting existing file '" + filename + "': " + e.getMessage());
            return false;
        }

        // Check if the system supports POSIX file permissions for an atomic operation.
        if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwxr-xr-x");
            try {
                Files.createFile(path, PosixFilePermissions.asFileAttribute(perms));
                System.out.println("Successfully created and set permissions for '" + filename + "' (POSIX).");
                return true;
            } catch (IOException e) {
                System.err.println("Error creating file '" + filename + "' with POSIX permissions: " + e.getMessage());
                return false;
            }
        } else {
            // Fallback for non-POSIX systems (e.g., Windows).
            File file = new File(filename);
            try {
                if (file.createNewFile()) {
                    // Attempt to set executable for owner, group, and others.
                    boolean success = file.setExecutable(true, false);
                    // Also ensure it's readable by all and writable by owner.
                    file.setReadable(true, false);
                    file.setWritable(true, true);
                    if (success) {
                        System.out.println("Successfully created and set executable flag for '" + filename + "' (Non-POSIX).");
                    } else {
                        System.out.println("Warning: Created '" + filename + "' but failed to set executable flag.");
                    }
                    return true;
                } else {
                    // This case should not be reached due to deleteIfExists above.
                    System.err.println("File '" + filename + "' already exists.");
                    return false;
                }
            } catch (IOException e) {
                System.err.println("Error creating file '" + filename + "' (Non-POSIX): " + e.getMessage());
                return false;
            }
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testFiles = {
            "script1.sh",
            "script2.sh",
            "script3.sh",
            "script4.sh",
            "script5.sh"
        };
        
        for (String filename : testFiles) {
            System.out.println("--- Testing: " + filename + " ---");
            createAndMakeExecutable(filename);
            System.out.println();
        }
    }
}