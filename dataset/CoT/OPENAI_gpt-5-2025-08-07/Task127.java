import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.*;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;

public class Task127 {

    // Step 1: Problem understanding and function definition
    // Securely copy a file from a specified base directory to a securely created temporary file.
    // Returns the path to the temp file as a String, or null on failure.
    public static String copyFileToTemp(String baseDir, String relativeFileName) {
        // Step 2: Security requirements - validate inputs
        if (baseDir == null || relativeFileName == null) {
            System.err.println("Error: invalid parameters.");
            return null;
        }

        try {
            // Resolve and validate base directory
            Path base = Paths.get(baseDir);
            if (!Files.exists(base) || !Files.isDirectory(base)) {
                System.err.println("Error: base directory not found or not a directory.");
                return null;
            }
            Path baseReal = base.toRealPath(LinkOption.NOFOLLOW_LINKS);

            // Prevent absolute path usage for the file name
            Path candidate = baseReal.resolve(relativeFileName).normalize();

            // Resolve the real path of the candidate (will fail if not exists)
            Path fileReal;
            try {
                fileReal = candidate.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                System.err.println("Error: source file not found.");
                return null;
            }

            // Ensure the file is within base directory (mitigate path traversal / symlink attacks)
            if (!fileReal.startsWith(baseReal)) {
                System.err.println("Error: access outside base directory is not allowed.");
                return null;
            }

            // Ensure it is a regular file
            if (!Files.isRegularFile(fileReal, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: specified path is not a regular file.");
                return null;
            }

            // Step 3: Secure coding generation - create secure temp file
            Path temp = Files.createTempFile("Task127_", ".tmp");
            // Set restrictive permissions if POSIX is available
            try {
                PosixFileAttributeView view = Files.getFileAttributeView(temp, PosixFileAttributeView.class);
                if (view != null) {
                    Files.setPosixFilePermissions(temp, EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE));
                }
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX systems: ignore
            }

            // Copy data using streams to avoid memory spikes for large files
            try (InputStream in = Files.newInputStream(fileReal, StandardOpenOption.READ);
                 OutputStream out = Files.newOutputStream(temp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                byte[] buffer = new byte[8192];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
            }

            // Step 4: Code review - handled errors, validated paths, safe temp creation, permissions set
            return temp.toString();

        } catch (SecurityException se) {
            System.err.println("Error: insufficient permissions.");
            return null;
        } catch (IOException ioe) {
            System.err.println("Error: I/O failure occurred.");
            return null;
        }
    }

    // Simple helper to create a file with content for tests
    private static void writeFile(Path dir, String name, byte[] content) throws IOException {
        Path p = dir.resolve(name);
        Files.createDirectories(p.getParent() == null ? dir : p.getParent());
        Files.write(p, content, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        // Step 5: Secure code output - run 5 test cases
        Path base = Files.createTempDirectory("Task127_test_java_");
        writeFile(base, "file1.txt", "Hello, secure world!".getBytes("UTF-8"));
        writeFile(base, "empty.txt", new byte[0]);
        Files.createDirectories(base.resolve("subdir"));
        writeFile(base, "subdir/nested.txt", "Nested content".getBytes("UTF-8"));

        // 1) Valid file copy
        System.out.println("Test1: " + copyFileToTemp(base.toString(), "file1.txt"));

        // 2) Non-existent file
        System.out.println("Test2: " + copyFileToTemp(base.toString(), "missing.txt"));

        // 3) Path traversal attempt
        System.out.println("Test3: " + copyFileToTemp(base.toString(), "../file1.txt"));

        // 4) Directory instead of file
        System.out.println("Test4: " + copyFileToTemp(base.toString(), "subdir"));

        // 5) Empty file copy
        System.out.println("Test5: " + copyFileToTemp(base.toString(), "empty.txt"));
    }
}