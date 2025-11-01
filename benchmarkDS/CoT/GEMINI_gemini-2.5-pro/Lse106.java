import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;
import java.util.HashSet;

public class Lse106 {

    /**
     * Creates a file, writes secret info to it, and sets permissions to owner-read-only.
     *
     * @param fileName   The name of the file to create.
     * @param secretInfo The secret string to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean createAndSecureFile(String fileName, String secretInfo) {
        Path filePath = Paths.get(fileName);
        try {
            // Write the super secret info to the file
            // The Files.write method handles opening and closing the file automatically.
            Files.write(filePath, secretInfo.getBytes());

            File file = filePath.toFile();
            
            // On POSIX-compliant systems (like Linux or macOS), set permissions to 0400.
            if (file.getFileSystem().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(filePath, perms);
            } else {
                // For non-POSIX systems (like Windows), use a fallback.
                // file.setReadOnly() makes the file read-only for everyone.
                // file.setWritable(false, true) makes it non-writable for the owner only.
                file.setReadable(true, true);
                file.setWritable(false, true);
                file.setExecutable(false, true);
            }

            System.out.println("Successfully created and secured file: " + fileName);
            return true;

        } catch (IOException | UnsupportedOperationException e) {
            System.err.println("Error processing file '" + fileName + "': " + e.getMessage());
            // Attempt to clean up partially created file on error
            try {
                Files.deleteIfExists(filePath);
            } catch (IOException cleanupEx) {
                System.err.println("Additionally, failed to clean up file '" + fileName + "': " + cleanupEx.getMessage());
            }
            return false;
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("--- Java Test Cases ---");
        createAndSecureFile("secret_alpha.txt", "The launch code is 1234.");
        createAndSecureFile("secret_beta.log", "Meeting at midnight.");
        createAndSecureFile("secret_gamma.dat", "Alpha protocol is a go.");
        createAndSecureFile("top_secret.conf", "Password=SuperSecret!@#");
        createAndSecureFile("confidential_notes.txt", "This is a test of the emergency secret system.");
        System.out.println("-----------------------\n");
    }
}