import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse105 {

    /**
     * Creates a file, writes content to it, and sets restrictive permissions.
     * On POSIX systems, permissions are set to owner-read/write only (600).
     * On other systems like Windows, it attempts a portable equivalent.
     *
     * @param filename The name of the file to create.
     * @param content The string content to write to the file.
     */
    public static void createAndSecureFile(String filename, String content) {
        Path path = Paths.get(filename);
        try {
            // 1. Write the string to the file, creating it if it doesn't exist.
            Files.write(path, content.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            System.out.println("Successfully created and wrote to " + filename);

            // 2. Change permissions to be restrictive.
            // This is the preferred method for POSIX-compliant systems (Linux, macOS).
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
                System.out.println("Set POSIX permissions for " + filename + " to owner-read/write.");
            } catch (UnsupportedOperationException e) {
                // Fallback for non-POSIX systems like Windows.
                java.io.File file = path.toFile();
                if (file.setReadable(false, false) && file.setWritable(false, false)) {
                     if (file.setReadable(true, true) && file.setWritable(true, true)) {
                        System.out.println("Set restrictive permissions for " + filename + " on non-POSIX system.");
                     }
                }
            }
        } catch (IOException e) {
            System.err.println("An error occurred with file '" + filename + "': " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        
        // 5 test cases, creating 5 different files.
        for (int i = 1; i <= 5; i++) {
            String filename = "secret" + i + ".txt";
            System.out.println("\n--- Test Case " + i + ": Creating " + filename + " ---");
            createAndSecureFile(filename, content);
        }
    }
}