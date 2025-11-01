import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse102 {

    /**
     * Opens or creates a file for reading and writing with specific POSIX permissions,
     * and writes content to it.
     *
     * @param filePath The path to the file.
     * @param content  The string content to write to the file.
     * @return true if the operation was successful, false otherwise.
     */
    public static boolean createFileWithPermissionsAndWrite(String filePath, String content) {
        Path path = Paths.get(filePath);
        
        // Define permissions: rw-r--r-- (0644)
        // S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
        Set<PosixFilePermission> perms = new HashSet<>();
        perms.add(PosixFilePermission.OWNER_READ);
        perms.add(PosixFilePermission.OWNER_WRITE);
        perms.add(PosixFilePermission.GROUP_READ);
        perms.add(PosixFilePermission.OTHERS_READ);
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);

        // Define open options: Read, Write, Create if not exists, Truncate on open
        Set<OpenOption> options = new HashSet<>();
        options.add(StandardOpenOption.CREATE);
        options.add(StandardOpenOption.WRITE);
        options.add(StandardOpenOption.READ);
        options.add(StandardOpenOption.TRUNCATE_EXISTING);

        try {
            // Check if the file system supports POSIX permissions
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                // On POSIX-compliant systems, create the file with permissions atomically
                try (SeekableByteChannel sbc = Files.newByteChannel(path, options, attr)) {
                    sbc.write(ByteBuffer.wrap(content.getBytes(StandardCharsets.UTF_8)));
                }
            } else {
                // Fallback for non-POSIX systems (like Windows)
                // Note: Permissions will be default for the OS.
                System.out.println("Warning: POSIX permissions not supported. File created with default permissions.");
                Files.write(path, content.getBytes(StandardCharsets.UTF_8), 
                            StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error creating or writing to file '" + filePath + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"important_config", "important_config"},
            {"test_java_1.cfg", "some_data_1"},
            {"test_java_2.cfg", "some_data_2"},
            {"test_java_3.cfg", "some_data_3"},
            {"test_java_4.cfg", "some_data_4"}
        };

        for (String[] testCase : testCases) {
            String filename = testCase[0];
            String content = testCase[1];
            
            System.out.println("--- Java Test Case: " + filename + " ---");
            boolean success = createFileWithPermissionsAndWrite(filename, content);
            System.out.println("Operation successful: " + success);

            if (success) {
                try {
                    // Clean up the created file
                    Files.deleteIfExists(Paths.get(filename));
                    System.out.println("Cleaned up file: " + filename);
                } catch (IOException e) {
                    System.err.println("Failed to clean up file: " + filename);
                }
            }
            System.out.println();
        }
    }
}