import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse105 {

    /**
     * Creates a file, writes content to it, and then sets restrictive permissions.
     *
     * @param filename The name of the file to create.
     * @param content  The string content to write to the file.
     * @return true if the operation was successful, false otherwise.
     */
    public static boolean createAndSecureFile(String filename, String content) {
        Path path = Paths.get(filename);
        try {
            // 1. Write the string to the file. This will create the file or overwrite it if it exists.
            Files.write(path, content.getBytes(StandardCharsets.UTF_8));
            System.out.println("Successfully wrote to " + filename);

            // 2. Change the permissions to be restrictive (owner read/write only).
            // This is platform-dependent.
            String os = System.getProperty("os.name").toLowerCase();
            if (os.contains("win")) {
                // On Windows, we use the File API to set owner-only permissions.
                File file = path.toFile();
                if (file.setReadable(false, false) && file.setReadable(true, true) &&
                    file.setWritable(false, false) && file.setWritable(true, true) &&
                    file.setExecutable(false, false)) {
                    System.out.println("Set restrictive permissions for " + filename + " on Windows.");
                } else {
                    System.err.println("Warning: Could not set all restrictive permissions for " + filename + " on Windows.");
                }
            } else {
                // On POSIX-compliant systems (Linux, macOS), we set permissions to 600.
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
                System.out.println("Set restrictive permissions (600) for " + filename + " on POSIX system.");
            }
            return true;
        } catch (UnsupportedOperationException e) {
            System.err.println("Error: Changing file permissions is not supported on this file system for " + filename + ". " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("An I/O error occurred with " + filename + ": " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("A security error prevented changing permissions for " + filename + ": " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        
        // 5 Test Cases: Create 5 separate files to demonstrate the function.
        for (int i = 1; i <= 5; i++) {
            String filename = "secret" + i + ".txt";
            System.out.println("\n--- Test Case " + i + ": Creating " + filename + " ---");
            boolean success = createAndSecureFile(filename, content);
            if (success) {
                System.out.println("Test Case " + i + " PASSED.");
            } else {
                System.out.println("Test Case " + i + " FAILED.");
            }
        }
    }
}