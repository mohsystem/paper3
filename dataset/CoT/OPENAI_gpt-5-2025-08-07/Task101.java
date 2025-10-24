import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermissions;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.FileAttribute;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

public class Task101 {

    // Step 1: Problem understanding:
    // Create a file named "script.sh" in a given directory with provided content, then make it executable.

    // Step 2: Security requirements:
    // - Avoid overwriting existing files (use CREATE_NEW).
    // - Avoid following symlinks for sensitive operations where possible.
    // - Set least-privilege permissions (700 on POSIX; executable on owner for non-POSIX).
    // - Safely write and sync content to disk.

    // Step 3: Secure coding generation:
    public static boolean createScriptFile(String directory, String content, boolean setExecutable) {
        if (directory == null || directory.isEmpty()) return false;
        if (content == null) content = "";
        Path dir = Paths.get(directory).normalize();
        Path target = dir.resolve("script.sh").normalize();

        // Ensure the target remains within the provided directory
        try {
            if (!target.getParent().toRealPath(LinkOption.NOFOLLOW_LINKS).equals(dir.toRealPath(LinkOption.NOFOLLOW_LINKS))) {
                return false;
            }
        } catch (IOException e) {
            return false;
        }

        try {
            // Create parent directories if not exist
            Files.createDirectories(dir);

            boolean posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
            FileAttribute<Set<PosixFilePermission>> attr = null;
            if (posix) {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE
                );
                if (setExecutable) {
                    perms.add(PosixFilePermission.OWNER_EXECUTE);
                }
                attr = PosixFilePermissions.asFileAttribute(perms);
            }

            // Create new file without overwriting
            if (attr != null) {
                Files.createFile(target, attr);
            } else {
                Files.createFile(target);
            }

            // Write content safely and fsync
            try (FileOutputStream fos = new FileOutputStream(target.toFile(), false)) {
                fos.write(content.getBytes(java.nio.charset.StandardCharsets.UTF_8));
                fos.flush();
                FileDescriptor fd = fos.getFD();
                fd.sync();
            }

            // If not POSIX or attribute could not set executable earlier, set now
            if (!posix && setExecutable) {
                File f = target.toFile();
                if (!f.setExecutable(true, true)) {
                    return false;
                }
            }

            // Validate executable if requested
            if (setExecutable) {
                try {
                    if (!Files.isExecutable(target)) {
                        // Attempt again for POSIX systems in case attributes were not applied initially
                        if (posix) {
                            Set<PosixFilePermission> perms = new HashSet<>(Files.getPosixFilePermissions(target, LinkOption.NOFOLLOW_LINKS));
                            perms.add(PosixFilePermission.OWNER_EXECUTE);
                            Files.setPosixFilePermissions(target, perms);
                        } else {
                            target.toFile().setExecutable(true, true);
                        }
                    }
                } catch (UnsupportedOperationException ignored) {
                    // Some FS may not support checking executability; consider success if no exception
                }
            }

            return true;
        } catch (FileAlreadyExistsException e) {
            return false;
        } catch (IOException e) {
            return false;
        }
    }

    // Step 4 and 5: Code review & Secure code output: integrated within above method.

    public static void main(String[] args) {
        // 5 test cases using temporary directories
        for (int i = 1; i <= 5; i++) {
            try {
                Path temp = Files.createTempDirectory("task101_java_test_" + i + "_");
                String content = "#!/bin/sh\necho \"Hello from Java test " + i + "\"\n";
                boolean result = createScriptFile(temp.toString(), content, true);
                Path script = temp.resolve("script.sh");
                System.out.println("Test " + i + " dir: " + temp);
                System.out.println("  Created: " + result);
                System.out.println("  Exists: " + Files.exists(script));
                System.out.println("  Executable: " + Files.isExecutable(script));
            } catch (IOException e) {
                System.out.println("Test " + i + " failed to create temp directory.");
            }
        }
    }
}