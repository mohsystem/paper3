import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.*;
import java.util.HashSet;
import java.util.Set;

public class Lse100 {

    /**
     * Creates a file with specific POSIX permissions (owner read/write).
     * If the file already exists, it considers it a success.
     * On non-POSIX systems, it creates the file with default permissions.
     *
     * @param fileName The name of the file to create.
     * @return true if the file was created or already exists, false otherwise.
     */
    public static boolean createConfigFile(String fileName) {
        Path path = Paths.get(fileName);

        // Check if the file already exists
        if (Files.exists(path)) {
            System.out.println("File '" + fileName + "' already exists.");
            return true;
        }

        try {
            // For POSIX-compliant systems (Linux, macOS), set specific permissions
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                Files.createFile(path, attr);
                System.out.println("Successfully created '" + fileName + "' with owner read/write permissions.");
            } else {
                // For non-POSIX systems like Windows, create with default permissions
                Files.createFile(path);
                System.out.println("Successfully created '" + fileName + "' with default permissions.");
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error creating file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String fileName = "important_config";
        String otherFileName = "another_config.txt";

        System.out.println("--- Test Case 1: Create file for the first time ---");
        createConfigFile(fileName);

        System.out.println("\n--- Test Case 2: Try to create the same file again ---");
        createConfigFile(fileName);

        System.out.println("\n--- Test Case 3: Delete the file ---");
        try {
            if (Files.deleteIfExists(Paths.get(fileName))) {
                System.out.println("Deleted '" + fileName + "' for re-testing.");
            }
        } catch (IOException e) {
            System.err.println("Error deleting file: " + e.getMessage());
        }

        System.out.println("\n--- Test Case 4: Create the file again after deletion ---");
        createConfigFile(fileName);

        System.out.println("\n--- Test Case 5: Create a different file ---");
        createConfigFile(otherFileName);

        // Cleanup
        try {
            System.out.println("\n--- Cleanup ---");
            Files.deleteIfExists(Paths.get(fileName));
            Files.deleteIfExists(Paths.get(otherFileName));
            System.out.println("Cleanup complete.");
        } catch (IOException e) {
            System.err.println("Error during cleanup: " + e.getMessage());
        }
    }
}