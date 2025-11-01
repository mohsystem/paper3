import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse101 {
    // Step 1-2: Implement functionality and apply secure defaults.
    public static boolean secureWrite(String filename, String content) {
        if (filename == null || content == null) return false;

        Path p = Paths.get(filename).normalize();
        boolean posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
        Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);

        try {
            // Step 3: Create with restrictive permissions when possible.
            if (!Files.exists(p)) {
                if (posix) {
                    FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                    Files.createFile(p, attr);
                } else {
                    Files.createFile(p);
                }
            }
            // Step 4: Enforce restrictive permissions on existing file (POSIX only).
            if (posix) {
                try {
                    Files.setPosixFilePermissions(p, perms);
                } catch (UnsupportedOperationException ignored) {
                }
            }
            // Step 5: Write content safely.
            try (OutputStream os = Files.newOutputStream(p, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
                byte[] data = content.getBytes(StandardCharsets.UTF_8);
                os.write(data);
                os.flush();
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        System.out.println(secureWrite("important_config", "important_config"));
        System.out.println(secureWrite("important_config", "important_config"));
        System.out.println(secureWrite("./important_config", "important_config"));
        System.out.println(secureWrite("important_config", "important_config"));
        System.out.println(secureWrite("important_config", "important_config"));
    }
}