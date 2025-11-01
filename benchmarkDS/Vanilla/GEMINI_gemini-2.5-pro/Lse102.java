import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse102 {

    /**
     * Opens/creates a file with specific POSIX permissions and writes content to it.
     * On non-POSIX systems, it creates the file with default permissions.
     *
     * @param fileName The name of the file to create/open.
     * @param content  The string content to write to the file.
     */
    public static void createAndWriteFile(String fileName, String content) {
        Path path = Paths.get(fileName);
        
        // Corresponds to O_RDWR | O_CREAT
        Set<OpenOption> options = new HashSet<>();
        options.add(StandardOpenOption.CREATE);
        options.add(StandardOpenOption.READ);
        options.add(StandardOpenOption.WRITE);

        // Check if the file system supports POSIX permissions (e.g., Linux, macOS)
        if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
            // Corresponds to S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH (rw-r--r--)
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            perms.add(PosixFilePermission.GROUP_READ);
            perms.add(PosixFilePermission.OTHERS_READ);
            FileAttribute<Set<PosixFilePermission>> attrs = PosixFilePermissions.asFileAttribute(perms);
            
            try (SeekableByteChannel sbc = Files.newByteChannel(path, options, attrs)) {
                ByteBuffer buffer = ByteBuffer.wrap(content.getBytes());
                sbc.write(buffer);
                System.out.println("Successfully wrote to " + fileName + " with POSIX permissions.");
            } catch (IOException e) {
                System.err.println("Error processing file " + fileName + ": " + e.getMessage());
            }
        } else {
            // Fallback for non-POSIX systems (e.g., Windows)
            try {
                Files.write(path, content.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.WRITE);
                System.out.println("Successfully wrote to " + fileName + " on a non-POSIX system.");
            } catch (IOException e) {
                 System.err.println("Error processing file " + fileName + ": " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        // Test Case 1: As per the prompt
        createAndWriteFile("important_config", "important_config");
        // Test Case 2: A different file and content
        createAndWriteFile("java_test.log", "Log entry from Java.");
        // Test Case 3: A file with empty content
        createAndWriteFile("java_empty.txt", "");
        // Test Case 4: Overwriting the first file
        createAndWriteFile("important_config", "new important_config content from Java");
        // Test Case 5: Another new file
        createAndWriteFile("java_data.json", "{\"key\": \"value\"}");
        System.out.println("-----------------------\n");
    }
}