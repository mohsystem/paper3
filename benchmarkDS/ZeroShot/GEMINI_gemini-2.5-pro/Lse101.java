import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse101 {

    /**
     * Creates a file and writes content to it with secure permissions.
     * On POSIX systems, permissions are set to owner-only read/write (600).
     * On non-POSIX systems (like Windows), default permissions are used when writing.
     * This approach is not fully atomic (file is created, then permissions set, then written to),
     * but represents a secure practice in Java.
     *
     * @param fileName The name of the file to write to.
     * @param content  The string content to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean writeSecureFile(String fileName, String content) {
        if (fileName == null || fileName.trim().isEmpty() || content == null) {
            System.err.println("Error: File name or content cannot be null or empty.");
            return false;
        }

        Path path = Paths.get(fileName);
        byte[] contentBytes = content.getBytes();

        try {
            // Write content to the file (creates or overwrites)
            Files.write(path, contentBytes);

            // Set secure permissions after writing
            // This is effective on POSIX-compliant file systems (Linux, macOS)
            if (path.getFileSystem().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
            }

            return true;

        } catch (IOException | UnsupportedOperationException e) {
            System.err.println("Error writing to file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        
        runTestCase("important_config", "important_config");
        runTestCase("user_settings.conf", "user=admin;level=5;");
        runTestCase("api_key.txt", "ABC-123-DEF-456");
        runTestCase("log_file.log", "Initial log entry.");
        runTestCase("", "some content"); // Invalid empty filename
    }

    private static void runTestCase(String fileName, String content) {
        System.out.println("Running test for file: '" + (fileName != null ? fileName : "null") + "'");
        boolean success = writeSecureFile(fileName, content);
        if (success) {
            System.out.println(" -> Success.");
        } else {
            System.out.println(" -> Failed.");
        }
        System.out.println();
    }
}