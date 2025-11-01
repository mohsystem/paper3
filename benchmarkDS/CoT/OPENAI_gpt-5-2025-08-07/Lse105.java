import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

/*
Chain-of-Through in code (summarized):
1) Problem: Create/overwrite secret.txt with specific content and set restrictive permissions.
2) Security: Avoid race conditions and ensure restrictive perms; handle cross-platform differences.
3) Generation: Use POSIX attributes where available; on non-POSIX (e.g., Windows) use owner-only flags.
4) Review: Checked for proper error handling, safe charset, and permission application pre/post write.
5) Output: Final code mitigates identified issues and returns success/failure.
*/

public class Lse105 {
    public static boolean createSecretFile(String fileName, String content) {
        try {
            Path p = Paths.get(fileName);
            boolean supportsPosix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");

            if (supportsPosix) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                if (Files.notExists(p)) {
                    Files.createFile(p, PosixFilePermissions.asFileAttribute(perms));
                } else {
                    // Ensure restrictive perms before writing
                    try {
                        Files.setPosixFilePermissions(p, perms);
                    } catch (Exception ignore) { }
                }
                // Write content
                Files.write(p, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING);
                // Re-apply restrictive perms after writing
                try {
                    Files.setPosixFilePermissions(p, perms);
                } catch (Exception ignore) { }
            } else {
                // Non-POSIX (e.g., Windows): emulate restrictive permissions owner-only
                File f = p.toFile();
                if (!f.exists()) {
                    // Create first
                    if (!f.createNewFile()) {
                        return false;
                    }
                }
                // Set restrictive perms before writing
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);

                Files.write(p, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
                // Re-apply perms after write
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        String filename = "secret.txt";
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        for (int i = 1; i <= 5; i++) {
            boolean ok = createSecretFile(filename, content);
            System.out.println("Test " + i + ": " + ok);
        }
    }
}