import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

public class Lse106 {
    public static String writeSecret(String secret) throws IOException {
        if (secret == null) {
            throw new IllegalArgumentException("secret cannot be null");
        }

        Path dir = Paths.get(System.getProperty("java.io.tmpdir"));
        Path file = Files.createTempFile(dir, "secret_", ".txt");

        // Try to set owner read/write before writing, then read-only after writing
        boolean posix = false;
        try {
            Set<PosixFilePermission> permsRW = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(file, permsRW);
            posix = true;
        } catch (UnsupportedOperationException ignored) {
            File f = file.toFile();
            // Remove all permissions, then grant owner read/write
            f.setReadable(false, false);
            f.setWritable(false, false);
            f.setExecutable(false, false);
            f.setReadable(true, true);
            f.setWritable(true, true);
        }

        // Write the secret
        Files.write(file, secret.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING);

        // Set to read-only for the owner
        try {
            if (posix) {
                Set<PosixFilePermission> permsR = EnumSet.of(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(file, permsR);
            } else {
                File f = file.toFile();
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(false, false);
                f.setReadable(true, true);
            }
        } catch (UnsupportedOperationException ignored) {
            // Fallback handled above
        }

        return file.toAbsolutePath().toString();
    }

    public static void main(String[] args) {
        String[] secrets = new String[] {
            "super secret info #1",
            "super secret info #2",
            "super secret info #3",
            "super secret info #4",
            "super secret info #5"
        };

        for (String s : secrets) {
            try {
                String path = writeSecret(s);
                System.out.println(path);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}