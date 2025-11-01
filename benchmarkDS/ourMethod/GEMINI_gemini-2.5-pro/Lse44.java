import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.EnumSet;

public class Lse44 {

    private static final String SAFE_DIR_PATH = "/safe/";
    private static final long MAX_FILE_SIZE = 1024 * 1024; // 1 MiB

    /**
     * Reads a file from a secure directory, preventing path traversal and other attacks.
     *
     * @param filename The name of the file to read. Must not contain path separators.
     * @return The content of the file as a byte array.
     * @throws IOException if an I/O error occurs.
     * @throws SecurityException if a security violation is detected.
     */
    public static byte[] readFileFromSafeDirectory(String filename) throws IOException, SecurityException {
        // Rule #3: Validate input
        if (filename == null || filename.isEmpty()) {
            throw new SecurityException("Filename cannot be null or empty.");
        }
        if (filename.contains("/") || filename.contains("\\")) {
            throw new SecurityException("Filename cannot contain path separators.");
        }

        Path safeDir = Paths.get(SAFE_DIR_PATH);
        Path requestedPath = safeDir.resolve(filename).normalize();

        // Rule #1: After normalization, check if path is still within the safe directory
        if (!requestedPath.startsWith(safeDir)) {
            throw new SecurityException("Path traversal attempt detected.");
        }

        // Rule #2: Open-then-validate pattern to avoid TOCTOU
        // O_NOFOLLOW equivalent is LinkOption.NOFOLLOW_LINKS
        try (FileChannel channel = FileChannel.open(requestedPath, 
                EnumSet.of(StandardOpenOption.READ),
                LinkOption.NOFOLLOW_LINKS)) {

            // Validate the opened handle (FileChannel)
            // fstat equivalent: check size and type
            if (channel.size() > MAX_FILE_SIZE) {
                throw new IOException("File size exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
            }

            BasicFileAttributes attrs = Files.readAttributes(requestedPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                 throw new IOException("Requested path is not a regular file.");
            }
            
            // Rule #5: Read data safely within buffer boundaries
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            try (InputStream inputStream = Channels.newInputStream(channel)) {
                byte[] buffer = new byte[8192];
                int bytesRead;
                long totalBytesRead = 0;
                while ((bytesRead = inputStream.read(buffer)) != -1) {
                    totalBytesRead += bytesRead;
                    if (totalBytesRead > MAX_FILE_SIZE) {
                         throw new IOException("File size exceeds limit during read.");
                    }
                    baos.write(buffer, 0, bytesRead);
                }
            }
            return baos.toByteArray();

        } catch (NoSuchFileException e) {
            throw new IOException("File not found: " + filename, e);
        } catch (AccessDeniedException e) {
            throw new SecurityException("Access denied to file: " + filename, e);
        }
    }

    public static void main(String[] args) {
        /*
         * To run these test cases, you need to set up the environment.
         * You may need root permissions to create /safe.
         *
         * sudo mkdir -p /safe/subdir
         * sudo sh -c 'echo "This is a valid file." > /safe/valid_file.txt'
         * sudo sh -c 'echo "This is outside the safe dir." > /outside_file.txt'
         * sudo ln -s /safe/valid_file.txt /safe/symlink_to_valid
         * sudo chmod 600 /safe/valid_file.txt
         */

        String[] testFilenames = {
            "valid_file.txt",                // 1. Should succeed
            "non_existent_file.txt",         // 2. Should fail (file not found)
            "../outside_file.txt",           // 3. Should fail (path traversal)
            "/etc/passwd",                   // 4. Should fail (contains slashes)
            "symlink_to_valid",              // 5. Should fail (is a symlink)
            "subdir"                         // 6. Should fail (is a directory)
        };

        for (String filename : testFilenames) {
            System.out.println("--- Testing: " + filename + " ---");
            try {
                byte[] content = readFileFromSafeDirectory(filename);
                System.out.println("Success! Read " + content.length + " bytes.");
                // System.out.println("Content: " + new String(content, java.nio.charset.StandardCharsets.UTF_8));
            } catch (IOException | SecurityException e) {
                System.out.println("Failed as expected: " + e.getMessage());
            }
        }
    }
}