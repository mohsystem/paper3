import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystemException;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;

public class Lse106 {
    // Writes secret to a file and sets permissions to read-only (owner). Returns the absolute file path.
    public static String writeSecretToReadOnlyFile(String secret, String preferredFilename) {
        if (secret == null) {
            throw new IllegalArgumentException("secret cannot be null");
        }

        Path path = null;
        boolean posixSupported = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
        try {
            if (preferredFilename != null && !preferredFilename.isBlank()) {
                path = Paths.get(preferredFilename).toAbsolutePath().normalize();
                Path parent = path.getParent();
                if (parent != null) {
                    Files.createDirectories(parent);
                }

                // Create with restrictive permissions initially
                if (posixSupported) {
                    Set<PosixFilePermission> perms600 = PosixFilePermissions.fromString("rw-------");
                    FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms600);
                    path = Files.createFile(path, attr);
                } else {
                    path = Files.createFile(path);
                    // On non-POSIX, best-effort: restrict as much as possible
                    File f = path.toFile();
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                    f.setExecutable(false, false);
                }
            } else {
                // Create a secure temp file
                if (posixSupported) {
                    Set<PosixFilePermission> perms600 = PosixFilePermissions.fromString("rw-------");
                    FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms600);
                    path = Files.createTempFile("secret_", ".txt", attr);
                } else {
                    path = Files.createTempFile("secret_", ".txt");
                    File f = path.toFile();
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                    f.setExecutable(false, false);
                }
            }

            // Write the secret securely
            try (OutputStream os = Files.newOutputStream(path, StandardOpenOption.WRITE)) {
                os.write(secret.getBytes(StandardCharsets.UTF_8));
                os.flush();
                os.getFD().sync();
            } catch (UnsupportedOperationException e) {
                // Fallback if FileDescriptor not available (rare)
                try (OutputStream os = Files.newOutputStream(path, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                    os.write(secret.getBytes(StandardCharsets.UTF_8));
                    os.flush();
                }
            }

            // Set read-only for owner
            if (posixSupported) {
                Set<PosixFilePermission> perms400 = PosixFilePermissions.fromString("r--------");
                Files.setPosixFilePermissions(path, perms400);
            } else {
                File f = path.toFile();
                // Best-effort on non-POSIX (e.g., Windows)
                f.setReadable(true, true);
                f.setWritable(false, false);
                f.setExecutable(false, false);
            }

            return path.toString();
        } catch (FileAlreadyExistsException e) {
            throw new RuntimeException("File already exists: " + e.getMessage(), e);
        } catch (IOException e) {
            throw new RuntimeException("I/O error: " + e.getMessage(), e);
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] secrets = new String[] {
            "TopSecret-Alpha-" + UUID.randomUUID(),
            "TopSecret-Bravo-" + UUID.randomUUID(),
            "TopSecret-Charlie-" + UUID.randomUUID(),
            "TopSecret-Delta-" + UUID.randomUUID(),
            "TopSecret-Echo-" + UUID.randomUUID()
        };

        // Test 1: auto temp file
        String f1 = writeSecretToReadOnlyFile(secrets[0], null);
        System.out.println(f1);

        // Test 2: specified file (unique)
        String f2Name = Paths.get(System.getProperty("java.io.tmpdir"), "secret_specified_" + System.nanoTime() + ".txt").toString();
        String f2 = writeSecretToReadOnlyFile(secrets[1], f2Name);
        System.out.println(f2);

        // Test 3: auto temp file
        String f3 = writeSecretToReadOnlyFile(secrets[2], "");
        System.out.println(f3);

        // Test 4: specified nested path
        String f4Name = Paths.get(System.getProperty("java.io.tmpdir"), "secdir_" + UUID.randomUUID(), "secret_" + System.nanoTime() + ".txt").toString();
        String f4 = writeSecretToReadOnlyFile(secrets[3], f4Name);
        System.out.println(f4);

        // Test 5: auto temp file
        String f5 = writeSecretToReadOnlyFile(secrets[4], null);
        System.out.println(f5);
    }
}