import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse104 {

    /**
     * Creates a file if it doesn't exist, opens it for reading/writing,
     * and writes content to it.
     * This method uses secure practices by setting restrictive file permissions on creation
     * and using try-with-resources to ensure the file channel is closed.
     *
     * @param filePath The path to the file.
     * @param content  The string content to write to the file.
     * @return true if successful, false otherwise.
     */
    public static boolean createAndWriteFile(String filePath, String content) {
        Path path = Paths.get(filePath);

        // Define options for opening the file: Read, Write, Create if not exists.
        Set<OpenOption> options = new HashSet<>();
        options.add(StandardOpenOption.CREATE);
        options.add(StandardOpenOption.WRITE);
        options.add(StandardOpenOption.READ);
        
        // Use a try-with-resources block to ensure the FileChannel is closed automatically.
        try {
             // Set restrictive file permissions (0600) on creation for POSIX systems.
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                
                try (FileChannel fileChannel = FileChannel.open(path, options, attr)) {
                    // Although FileChannel.open will fail on a directory, this check ensures
                    // we are only operating on regular files, adhering to security best practices.
                    if (!Files.isRegularFile(path)) {
                        System.err.println("Error: Path does not point to a regular file: " + filePath);
                        // Clean up the created entry if it wasn't a regular file.
                        Files.deleteIfExists(path);
                        return false;
                    }

                    ByteBuffer buffer = ByteBuffer.wrap(content.getBytes(StandardCharsets.UTF_8));
                    while (buffer.hasRemaining()) {
                        fileChannel.write(buffer);
                    }
                }
            } else {
                 // For non-POSIX systems, create without specific permissions.
                 // The OS default permissions will apply.
                 try (FileChannel fileChannel = FileChannel.open(path, options)) {
                    if (!Files.isRegularFile(path)) {
                        System.err.println("Error: Path does not point to a regular file: " + filePath);
                        Files.deleteIfExists(path);
                        return false;
                    }
                    ByteBuffer buffer = ByteBuffer.wrap(content.getBytes(StandardCharsets.UTF_8));
                    while (buffer.hasRemaining()) {
                        fileChannel.write(buffer);
                    }
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String importantConfig = "important_config=true";

        // Test cases
        String[] testFiles = {
            "test_config1.txt",
            "test_config2.txt",
            "test_config3.txt",
            "test_config4.txt",
            "test_config5.txt"
        };
        
        for (String fileName : testFiles) {
            System.out.println("Attempting to write to: " + fileName);
            if (createAndWriteFile(fileName, importantConfig)) {
                System.out.println("Successfully wrote to " + fileName);
                try {
                    String content = new String(Files.readAllBytes(Paths.get(fileName)), StandardCharsets.UTF_8);
                    System.out.println("Content of " + fileName + ": \"" + content + "\"");
                    Files.delete(Paths.get(fileName)); // Clean up the test file
                    System.out.println("Cleaned up " + fileName);
                } catch (IOException e) {
                    System.err.println("Failed to read or clean up " + fileName + ": " + e.getMessage());
                }
            } else {
                System.err.println("Failed to write to " + fileName);
            }
            System.out.println("--------------------");
        }
    }
}