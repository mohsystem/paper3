import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {

    /**
     * Creates a file, writes content to it, sets restrictive permissions,
     * and prints the final permissions.
     * This implementation is primarily for POSIX-compliant file systems.
     *
     * @param filename The name of the file to create.
     * @param content  The string content to write to the file.
     */
    public static void handleSecretFile(String filename, String content) {
        Path path = Paths.get(filename);
        System.out.println("Processing file: " + filename);

        try {
            // 1. Create a file and write the string to it.
            // This will create the file with default permissions.
            Files.writeString(path, content);
            System.out.println("  - Successfully wrote to " + filename);

            // 2. Change the permissions of the file to be restrictive (owner read/write only).
            // This is the POSIX way. It will fail on non-POSIX systems like Windows FAT32.
            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(path, perms);
                System.out.println("  - Set permissions to rw-------");

                // 3. Print the permissions of the file.
                Set<PosixFilePermission> finalPerms = Files.getPosixFilePermissions(path);
                System.out.println("  - Final permissions: " + PosixFilePermissions.toString(finalPerms));

            } catch (UnsupportedOperationException e) {
                // Handle non-POSIX systems gracefully
                System.out.println("  - POSIX file permissions not supported on this system. Trying Windows ACLs.");
                // For Windows, making a file read-only is a common restrictive action.
                // Note: This is not equivalent to POSIX 600.
                path.toFile().setWritable(false, false); // All users cannot write
                path.toFile().setReadable(false, false); // All users cannot read
                path.toFile().setReadable(true, true);   // Owner can read
                path.toFile().setWritable(true, true);   // Owner can write
                System.out.println("  - Set file to readable/writable by owner only (Windows).");
                boolean canRead = path.toFile().canRead();
                boolean canWrite = path.toFile().canWrite();
                System.out.println("  - Final permissions: Owner can read: " + canRead + ", Owner can write: " + canWrite);
            }

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        } finally {
            System.out.println("------------------------------------");
        }
    }

    public static void main(String[] args) {
        final String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

        // 5 Test Cases
        handleSecretFile("secret1.txt", content);
        handleSecretFile("secret2.txt", content);
        handleSecretFile("secret3.txt", content);
        handleSecretFile("secret4.txt", content);
        handleSecretFile("secret5.txt", content);
    }
}