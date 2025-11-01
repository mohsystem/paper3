import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse102 {

    /**
     * Creates a file if it doesn't exist with specific POSIX permissions,
     * opens it for reading/writing, and writes content to it.
     * This method uses NIO and is designed to be secure against TOCTOU vulnerabilities
     * by using atomic options.
     *
     * @param filename The name of the file to open/create.
     * @param content  The string content to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean createAndWriteFile(String filename, String content) {
        Path path = Paths.get(filename);
        byte[] contentBytes = content.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.wrap(contentBytes);

        // Define options for opening the file
        Set<OpenOption> options = new HashSet<>();
        options.add(StandardOpenOption.CREATE);
        options.add(StandardOpenOption.WRITE);
        options.add(StandardOpenOption.READ);
        // O_NOFOLLOW equivalent: prevents following symbolic links
        options.add(StandardOpenOption.NOFOLLOW_LINKS);

        // Define permissions: rw-r--r-- (0644)
        Set<PosixFilePermission> perms = new HashSet<>();
        perms.add(PosixFilePermission.OWNER_READ);
        perms.add(PosixFilePermission.OWNER_WRITE);
        perms.add(PosixFilePermission.GROUP_READ);
        perms.add(PosixFilePermission.OTHERS_READ);
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);

        // Use try-with-resources to ensure the channel is closed automatically
        try {
            FileChannel channel;
            // Check if the file system supports POSIX attributes
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                channel = FileChannel.open(path, options, attr);
            } else {
                // For non-POSIX systems like Windows, open without attributes
                channel = FileChannel.open(path, options);
            }

            try (FileChannel fc = channel) {
                fc.truncate(0); // Clear the file in case it already exists and has content
                while (buffer.hasRemaining()) {
                    fc.write(buffer);
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error processing file '" + filename + "': " + e.getMessage());
            return false;
        } catch (UnsupportedOperationException e) {
             System.err.println("Error: POSIX file permissions not supported on this file system for '" + filename + "'.");
             return false;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: The primary specified case
        System.out.println("Test Case 1: important_config");
        if (createAndWriteFile("important_config", "important_config")) {
            System.out.println("  Success.");
        } else {
            System.out.println("  Failure.");
        }

        // Test Case 2: A different filename and content
        System.out.println("\nTest Case 2: another_config.txt");
        if (createAndWriteFile("another_config.txt", "some other data")) {
            System.out.println("  Success.");
        } else {
            System.out.println("  Failure.");
        }

        // Test Case 3: Empty content
        System.out.println("\nTest Case 3: empty_file.cfg");
        if (createAndWriteFile("empty_file.cfg", "")) {
            System.out.println("  Success.");
        } else {
            System.out.println("  Failure.");
        }

        // Test Case 4: File with special characters in name (if OS supports)
        System.out.println("\nTest Case 4: file-with-hyphen.log");
        if (createAndWriteFile("file-with-hyphen.log", "log entry 1")) {
            System.out.println("  Success.");
        } else {
            System.out.println("  Failure.");
        }

        // Test Case 5: Overwriting an existing file
        System.out.println("\nTest Case 5: Overwriting important_config");
        if (createAndWriteFile("important_config", "new content")) {
            System.out.println("  Success.");
        } else {
            System.out.println("  Failure.");
        }
    }
}