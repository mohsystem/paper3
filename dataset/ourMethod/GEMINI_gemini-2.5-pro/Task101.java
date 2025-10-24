import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Task101 {

    /**
     * Creates a file with the given name, writes a simple shell script to it,
     * and makes it executable for the owner.
     * This implementation uses POSIX file permissions for security and is intended
     * for POSIX-compliant file systems.
     *
     * @param fileName The name of the file to create. Must not contain path separators.
     * @return true if the file was created and made executable, false otherwise.
     */
    public static boolean createAndMakeExecutable(String fileName) {
        if (fileName == null || fileName.isEmpty() || fileName.contains("/") || fileName.contains("\\")) {
            System.err.println("Error: Invalid filename. It must not be empty or contain path separators.");
            return false;
        }

        // Check for POSIX support for setting permissions securely at creation
        if (!FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
            System.err.println("Error: POSIX file attributes not supported on this system.");
            return false;
        }

        Path path = Paths.get(fileName);
        String content = "#!/bin/sh\necho \"Hello from " + fileName + "!\"\n";

        // Define file permissions: rwxr-xr-x (755)
        Set<PosixFilePermission> perms = new HashSet<>();
        perms.add(PosixFilePermission.OWNER_READ);
        perms.add(PosixFilePermission.OWNER_WRITE);
        perms.add(PosixFilePermission.OWNER_EXECUTE);
        perms.add(PosixFilePermission.GROUP_READ);
        perms.add(PosixFilePermission.GROUP_EXECUTE);
        perms.add(PosixFilePermission.OTHERS_READ);
        perms.add(PosixFilePermission.OTHERS_EXECUTE);

        try {
            // Delete file if it exists to ensure a clean state for the example
            Files.deleteIfExists(path);
            
            // Create file and write content
            try (BufferedWriter writer = Files.newBufferedWriter(path, StandardCharsets.UTF_8)) {
                writer.write(content);
            }
            
            // Set permissions
            Files.setPosixFilePermissions(path, perms);

            return true;
        } catch (IOException | UnsupportedOperationException e) {
            System.err.println("An error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testFilenames = {
            "script1.sh",
            ".hidden_script.sh",
            "no_extension",
            "",                      // Invalid: empty
            "test/script.sh"       // Invalid: contains path separator
        };

        for (String filename : testFilenames) {
            System.out.println("Testing with filename: \"" + filename + "\"");
            boolean success = createAndMakeExecutable(filename);
            System.out.println("Result: " + (success ? "SUCCESS" : "FAILURE"));
            if (success) {
                System.out.println("File '" + filename + "' created and made executable.");
                // Clean up the created file
                try {
                    Files.delete(Paths.get(filename));
                    System.out.println("Cleaned up '" + filename + "'.");
                } catch (IOException e) {
                    System.err.println("Failed to clean up file '" + filename + "': " + e.getMessage());
                }
            }
            System.out.println("--------------------");
        }
    }
}