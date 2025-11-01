import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse107 {

    /**
     * Creates a file, writes content, sets restrictive permissions, and prints them.
     * @param filename The name of the file to process.
     * @param content The string content to write to the file.
     */
    public static void manageSecretFile(String filename, String content) {
        Path filePath = Paths.get(filename);
        System.out.println("Processing file: " + filename);
        try {
            // 1. Create a file and write the string to it.
            Files.write(filePath, content.getBytes());
            System.out.println("  - Successfully wrote to " + filename);

            // 2. Change the permissions of the file to restrictive (owner read/write only).
            // This part is POSIX-specific (for Linux, macOS, etc.)
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(filePath, perms);
                System.out.println("  - Set permissions to rw------- (600)");

                // 3. Print the permissions of the file.
                Set<PosixFilePermission> currentPerms = Files.getPosixFilePermissions(filePath);
                System.out.println("  - Current permissions: " + PosixFilePermissions.toString(currentPerms));

            } catch (UnsupportedOperationException e) {
                // Handle non-POSIX systems like Windows
                System.out.println("  - POSIX permissions not supported. Using basic permissions.");
                File file = filePath.toFile();
                file.setReadable(false, false); // Clear all read permissions
                file.setWritable(false, false); // Clear all write permissions
                file.setReadable(true, true);   // Set read for owner only
                file.setWritable(true, true);   // Set write for owner only
                System.out.println("  - Set permissions to owner-only read/write.");
                System.out.println("  - Current permissions: Readable=" + file.canRead() + ", Writable=" + file.canWrite());
            }

        } catch (IOException e) {
            System.err.println("An error occurred while processing " + filename + ": " + e.getMessage());
        } finally {
            // Clean up the created file for the next test case
            try {
                Files.deleteIfExists(filePath);
                System.out.println("  - Cleaned up file: " + filename);
            } catch (IOException e) {
                // ignore cleanup errors
            }
            System.out.println("--------------------");
        }
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        
        // Test Case 1
        manageSecretFile("secret.txt", content);

        // Test Case 2
        manageSecretFile("secret_file_2.txt", content);

        // Test Case 3
        manageSecretFile("another_secret.log", content);
        
        // Test Case 4
        manageSecretFile("temp_secret_4.dat", content);
        
        // Test Case 5
        manageSecretFile("last_secret.txt", content);
    }
}