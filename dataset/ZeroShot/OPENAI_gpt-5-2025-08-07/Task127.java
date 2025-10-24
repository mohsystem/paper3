import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Task127 {
    public static String safeCopyToTemp(String baseDir, String relativePath, String tempDir) {
        if (baseDir == null || relativePath == null) {
            System.err.println("Invalid arguments: baseDir and relativePath must not be null.");
            return null;
        }
        try {
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            if (!Files.exists(base) || !Files.isDirectory(base)) {
                System.err.println("Base directory does not exist or is not a directory.");
                return null;
            }
            Path baseReal;
            try {
                baseReal = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                System.err.println("Unable to resolve base directory: " + e.getMessage());
                return null;
            }

            Path candidate = base.resolve(relativePath).normalize();
            Path candidateReal;
            try {
                candidateReal = candidate.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                System.err.println("Source file cannot be resolved: " + e.getMessage());
                return null;
            }

            if (!candidateReal.startsWith(baseReal)) {
                System.err.println("Security violation: Attempted path traversal outside base directory.");
                return null;
            }

            if (!Files.isRegularFile(candidateReal, LinkOption.NOFOLLOW_LINKS) || !Files.isReadable(candidateReal)) {
                System.err.println("Source is not a regular readable file.");
                return null;
            }

            Path tempBase = (tempDir != null && !tempDir.isEmpty())
                    ? Paths.get(tempDir).toAbsolutePath().normalize()
                    : Paths.get(System.getProperty("java.io.tmpdir")).toAbsolutePath().normalize();
            try {
                Files.createDirectories(tempBase);
            } catch (IOException e) {
                System.err.println("Unable to create/access temp directory: " + e.getMessage());
                return null;
            }

            Path tempFile;
            try {
                tempFile = Files.createTempFile(tempBase, "safe_copy_", ".tmp");
            } catch (IOException e) {
                System.err.println("Unable to create temporary file: " + e.getMessage());
                return null;
            }

            // Set restrictive permissions if supported (POSIX)
            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(tempFile, perms);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX FS; ignore
            } catch (IOException e) {
                System.err.println("Warning: Unable to set file permissions: " + e.getMessage());
            }

            try (InputStream in = Files.newInputStream(candidateReal, StandardOpenOption.READ);
                 OutputStream out = Files.newOutputStream(tempFile, StandardOpenOption.WRITE)) {
                byte[] buffer = new byte[8192];
                int len;
                while ((len = in.read(buffer)) != -1) {
                    out.write(buffer, 0, len);
                }
                out.flush();
            } catch (IOException e) {
                try { Files.deleteIfExists(tempFile); } catch (IOException ignored) {}
                System.err.println("IO error during copy: " + e.getMessage());
                return null;
            }

            return tempFile.toString();
        } catch (SecurityException se) {
            System.err.println("Security manager denied operation: " + se.getMessage());
            return null;
        }
    }

    private static void writeFile(Path p, String content) throws IOException {
        Files.createDirectories(p.getParent());
        Files.write(p, content.getBytes("UTF-8"), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        Path testBase = Files.createTempDirectory("task127_java_base_");
        Path a = testBase.resolve("a.txt");
        Path sub = testBase.resolve("sub");
        Path b = sub.resolve("b.txt");
        writeFile(a, "Hello A");
        writeFile(b, "Hello B");

        // 5 Test cases
        String r1 = safeCopyToTemp(testBase.toString(), "a.txt", null);
        System.out.println("Test1: " + r1);

        String r2 = safeCopyToTemp(testBase.toString(), "sub/b.txt", null);
        System.out.println("Test2: " + r2);

        String r3 = safeCopyToTemp(testBase.toString(), "../../etc/passwd", null);
        System.out.println("Test3: " + r3);

        String r4 = safeCopyToTemp(testBase.toString(), "missing.txt", null);
        System.out.println("Test4: " + r4);

        String r5 = safeCopyToTemp(a.toString(), "a.txt", null); // baseDir is a file, not a dir
        System.out.println("Test5: " + r5);
    }
}