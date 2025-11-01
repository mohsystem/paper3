import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse102 {

    /**
     * Creates and writes to a file with specified POSIX permissions (0644).
     * The file is opened for reading and writing, created if it doesn't exist,
     * and truncated if it does.
     *
     * @param fileName The name of the file to create/open.
     * @param content  The content to write to the file.
     * @return true if successful, false otherwise.
     */
    public static boolean createAndWriteFile(String fileName, String content) {
        Path path = Paths.get(fileName);
        byte[] contentBytes = content.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.wrap(contentBytes);

        // Define permissions: Owner read/write, Group read, Others read (0644)
        Set<PosixFilePermission> perms = EnumSet.of(
            PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE,
            PosixFilePermission.GROUP_READ, PosixFilePermission.OTHERS_READ
        );
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);

        // Define open options: Read, Write, Create, Truncate
        Set<StandardOpenOption> options = EnumSet.of(
            StandardOpenOption.READ, StandardOpenOption.WRITE,
            StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING
        );
        
        // Use a try-with-resources block to ensure the channel is closed automatically
        try {
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                // On POSIX-compliant systems, create with specific permissions
                try (FileChannel fc = FileChannel.open(path, options, attr)) {
                    fc.write(buffer);
                }
            } else {
                // Fallback for non-POSIX systems (like Windows)
                System.err.println("Warning: POSIX permissions not supported. Creating with default permissions.");
                try (FileChannel fc = FileChannel.open(path, options)) {
                    fc.write(buffer);
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error creating or writing to file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String fileName = "important_config";
        String content = "important_config";

        System.out.println("Running 5 test cases...");
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            System.out.println("Attempting to write '" + content + "' to '" + fileName + "'");
            boolean success = createAndWriteFile(fileName, content);
            if (success) {
                System.out.println("Successfully wrote to file.");
                // Optional: Read back to verify content
                try {
                    String readContent = new String(Files.readAllBytes(Paths.get(fileName)), StandardCharsets.UTF_8);
                    System.out.println("File content verified: \"" + readContent + "\"");
                } catch (IOException e) {
                    System.err.println("Failed to read back file for verification: " + e.getMessage());
                }
            } else {
                System.err.println("Failed to write to file.");
            }
        }
    }
}