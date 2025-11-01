import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {

    private static final String CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    /**
     * Creates a file, writes content, sets restrictive permissions, and prints them.
     * On POSIX systems, it creates the file and sets permissions atomically.
     * On other systems (like Windows), it creates the file then attempts to set
     * owner-only read/write permissions as a separate step.
     *
     * @param filename The name of the file to process.
     */
    public static void processFile(String filename) {
        Path path = Paths.get(filename);
        System.out.println("Processing file: " + filename);

        try {
            // Ensure file does not exist from a previous failed run
            Files.deleteIfExists(path);

            String os = System.getProperty("os.name").toLowerCase();

            // Check if the filesystem supports POSIX permissions
            if (Files.getFileStore(Paths.get(".")).supportsFileAttributeView("posix")) {
                // POSIX-compliant system
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                // Create file and set permissions atomically to avoid race conditions
                Files.createFile(path, PosixFilePermissions.asFileAttribute(perms));
                System.out.println("File created with POSIX permissions: rw-------");
                
                Files.writeString(path, CONTENT, StandardOpenOption.WRITE);
                System.out.println("Content written to file.");

                Set<PosixFilePermission> actualPerms = Files.getPosixFilePermissions(path);
                System.out.println("Current file permissions: " + PosixFilePermissions.toString(actualPerms));

            } else {
                // Non-POSIX system (e.g., Windows with NTFS)
                Files.writeString(path, CONTENT);
                System.out.println("File created and content written.");
                
                java.io.File file = path.toFile();
                if (file.setReadable(false, false) && file.setWritable(false, false) && file.setExecutable(false, false)) {
                     if (file.setReadable(true, true) && file.setWritable(true, true)) {
                        System.out.println("Permissions set to restrictive (owner-only read/write).");
                     } else {
                        System.out.println("Warning: Could not set owner-only read/write permissions.");
                     }
                } else {
                    System.out.println("Warning: Could not clear all existing permissions.");
                }

                System.out.println("Current permissions check (for current process): " +
                    "Readable=" + file.canRead() + ", Writable=" + file.canWrite());
            }

        } catch (UnsupportedOperationException e) {
            System.err.println("Error: The file system does not support the required permission-setting operations. " + e.getMessage());
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            throw new UncheckedIOException(e);
        } finally {
            System.out.println("---");
        }
    }

    private static void cleanup(String filename) {
        try {
            Files.deleteIfExists(Paths.get(filename));
        } catch (IOException e) {
            System.err.println("Failed to clean up file " + filename + ": " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            String testFileName = "secret_java_" + i + ".txt";
            try {
                processFile(testFileName);
            } catch (Exception e) {
                System.err.println("Test case " + i + " failed for " + testFileName + ": " + e.getMessage());
            } finally {
                cleanup(testFileName);
            }
        }
    }
}