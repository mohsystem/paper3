import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Comparator;
import java.util.EnumSet;

public class Lse43 {
    // For this example, we create a 'safe' directory inside the system's temp folder
    // to ensure the code can run without special permissions. A real application would
    // use a fixed, secured path like "/safe".
    private static final Path SAFE_DIRECTORY_BASE = Paths.get(System.getProperty("java.io.tmpdir"));
    private static final Path SAFE_DIRECTORY = SAFE_DIRECTORY_BASE.resolve("safe_java").toAbsolutePath();

    private static final int MAX_FILE_SIZE = 1024 * 1024; // 1 MB limit
    private static final int BUFFER_SIZE = 8192;

    /**
     * Securely reads a file from a designated safe directory, following the
     * "open, then validate handle" principle as closely as possible with standard Java NIO.
     *
     * @param filename The name of the file to read. Must not contain path separators.
     * @return The content of the file as a byte array, or null on failure.
     */
    public static byte[] readFileFromSafeDirectory(String filename) {
        // Rule #4, #7: Input validation. Reject null/empty and path separators.
        if (filename == null || filename.isEmpty() || filename.contains("/") || filename.contains("\\") || filename.equals("..") || filename.equals(".")) {
            System.err.println("Error: Invalid filename provided.");
            return null;
        }

        Path filePath = SAFE_DIRECTORY.resolve(filename);

        // Rule #4: Normalize and check that the path is within the safe directory.
        if (!filePath.normalize().startsWith(SAFE_DIRECTORY)) {
            System.err.println("Error: Path traversal attempt detected.");
            return null;
        }

        // Rule #3, #5: Use try-with-resources to ensure the channel is closed.
        try (SeekableByteChannel channel = Files.newByteChannel(filePath, EnumSet.of(StandardOpenOption.READ), LinkOption.NOFOLLOW_LINKS)) {
            // The handle (channel) is now open. We validate it before use.
            // newByteChannel with NOFOLLOW_LINKS will fail on symlinks.
            // It will also fail on directories, effectively checking for a file-like object.

            // Rule #1: Validate size on the opened handle.
            if (channel.size() > MAX_FILE_SIZE) {
                System.err.println("Error: File is too large: " + filename);
                return null;
            }

            if (channel.size() == 0) {
                return new byte[0];
            }

            int capacity = (int) channel.size();
            ByteArrayOutputStream out = new ByteArrayOutputStream(capacity);
            ByteBuffer buffer = ByteBuffer.allocate(BUFFER_SIZE);
            long totalBytesRead = 0;

            while (channel.read(buffer) > 0) {
                buffer.flip();
                totalBytesRead += buffer.remaining();
                // Rule #1: Check size on each read to prevent TOCTOU file growth attacks
                if (totalBytesRead > MAX_FILE_SIZE) {
                    System.err.println("Error: File size exceeds limit during read.");
                    return null;
                }
                // Copy buffer content to the output stream
                byte[] chunk = new byte[buffer.remaining()];
                buffer.get(chunk);
                out.write(chunk);
                buffer.clear();
            }
            return out.toByteArray();

        } catch (NoSuchFileException e) {
            System.err.println("Error: File not found: " + filename);
        } catch (AccessDeniedException e) {
            System.err.println("Error: Access denied to file: " + filename);
        } catch (FileSystemException e) {
            System.err.println("Error: Not a regular file (is it a directory or symlink?): " + filename);
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Error: Security manager prevented file access: " + e.getMessage());
        }

        return null;
    }
    
    public static void setupTestEnvironment() {
        try {
            if (Files.exists(SAFE_DIRECTORY)) {
                // Naive recursive delete for test cleanup
                Files.walk(SAFE_DIRECTORY)
                    .sorted(Comparator.reverseOrder())
                    .forEach(p -> {
                        try { Files.delete(p); } catch (IOException e) { /* ignore */ }
                    });
            }
            Files.createDirectories(SAFE_DIRECTORY);

            Path goodFile = SAFE_DIRECTORY.resolve("goodfile.txt");
            Files.write(goodFile, "hello world".getBytes(StandardCharsets.UTF_8));

            Path outsideFile = SAFE_DIRECTORY_BASE.resolve("outside.txt");
            Files.write(outsideFile, "secret".getBytes(StandardCharsets.UTF_8));

            Path symlink = SAFE_DIRECTORY.resolve("symlink.txt");
            Files.deleteIfExists(symlink);
            Files.createSymbolicLink(symlink, outsideFile);

            Path subdir = SAFE_DIRECTORY.resolve("subdir");
            Files.createDirectories(subdir);
        } catch (IOException | SecurityException e) {
            System.err.println("Warning: Failed to set up test environment: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        setupTestEnvironment();
        System.out.println("--- Running Test Cases ---");
        System.out.println("Using safe directory: " + SAFE_DIRECTORY);

        System.out.println("\n1. Reading 'goodfile.txt' (should succeed):");
        byte[] content = readFileFromSafeDirectory("goodfile.txt");
        if (content != null) {
            System.out.println("Success! Content: " + new String(content, StandardCharsets.UTF_8));
        } else {
            System.out.println("Failed.");
        }

        System.out.println("\n2. Reading 'nonexistent.txt' (should fail):");
        content = readFileFromSafeDirectory("nonexistent.txt");
        if (content == null) {
            System.out.println("Success! Read failed as expected.");
        } else {
            System.out.println("Failed. Read should not have succeeded.");
        }

        System.out.println("\n3. Reading 'symlink.txt' (should fail):");
        content = readFileFromSafeDirectory("symlink.txt");
        if (content == null) {
            System.out.println("Success! Symlink read failed as expected.");
        } else {
            System.out.println("Failed. Symlink should have been rejected.");
        }
        
        System.out.println("\n4. Reading '../.../goodfile.txt' (should fail):");
        String traversalPath = Paths.get("..", SAFE_DIRECTORY.getFileName().toString(), "goodfile.txt").toString();
        content = readFileFromSafeDirectory(traversalPath);
        if (content == null) {
            System.out.println("Success! Path traversal failed as expected.");
        } else {
            System.out.println("Failed. Path traversal should have been blocked.");
        }

        System.out.println("\n5. Reading 'subdir' (should fail):");
        content = readFileFromSafeDirectory("subdir");
        if (content == null) {
            System.out.println("Success! Reading a directory failed as expected.");
        } else {
            System.out.println("Failed. Directory should not be readable as a file.");
        }
    }
}