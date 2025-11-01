import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse104 {

    /**
     * Opens or creates a file for writing and writes content to it.
     * This method emulates the behavior of open(O_RDWR | O_CREAT).
     *
     * @param fileName The name of the file to write to.
     * @param content  The string content to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean secureWriteToFile(String fileName, String content) {
        if (fileName == null || fileName.trim().isEmpty() || content == null) {
            System.err.println("Error: File name and content cannot be null or empty.");
            return false;
        }

        Path path = Paths.get(fileName);
        byte[] contentBytes = content.getBytes();

        try {
            // Write to the file, creating it if it doesn't exist.
            // WRITE implies write access.
            // CREATE will create the file if it does not exist.
            // TRUNCATE_EXISTING is not used, so existing content at the beginning will be overwritten.
            Files.write(path, contentBytes, StandardOpenOption.CREATE, StandardOpenOption.WRITE);
            
            // Set secure permissions (owner read/write only) on non-Windows systems
            try {
                String os = System.getProperty("os.name").toLowerCase();
                if (!os.contains("win")) {
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    Files.setPosixFilePermissions(path, perms);
                }
            } catch (UnsupportedOperationException e) {
                // Filesystem does not support POSIX permissions, can be ignored.
            }

            return true;
        } catch (IOException e) {
            System.err.println("Error writing to file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Create a new file and write content.
        System.out.println("Test 1: " + (secureWriteToFile("config1.txt", "important_config") ? "SUCCESS" : "FAILURE"));

        // Test Case 2: Create another file with different content.
        System.out.println("Test 2: " + (secureWriteToFile("config2.txt", "user=admin;password=secret") ? "SUCCESS" : "FAILURE"));

        // Test Case 3: Overwrite the beginning of the first file.
        System.out.println("Test 3: " + (secureWriteToFile("config1.txt", "new_config") ? "SUCCESS" : "FAILURE"));

        // Test Case 4: Attempt to write to a file in a non-existent directory (should fail).
        System.out.println("Test 4: " + (secureWriteToFile("non_existent_dir/config.txt", "data") ? "SUCCESS" : "FAILURE"));

        // Test Case 5: Write an empty string to a file.
        System.out.println("Test 5: " + (secureWriteToFile("config5.txt", "") ? "SUCCESS" : "FAILURE"));
    }
}