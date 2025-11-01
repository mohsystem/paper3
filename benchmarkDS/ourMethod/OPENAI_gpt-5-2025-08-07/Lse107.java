import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

public class Lse107 {
    private static final String CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    private static final String TARGET_NAME = "secret.txt";

    public static void main(String[] args) {
        try {
            // 5 test cases using 5 temporary directories
            for (int i = 0; i < 5; i++) {
                Path tempDir = Files.createTempDirectory("safe_dir_");
                String result = createSecretAndPrintPerms(tempDir.toString());
                System.out.println("Base: " + tempDir + " -> " + result);
            }
        } catch (IOException e) {
            System.out.println("Error");
        }
    }

    public static String createSecretAndPrintPerms(String baseDir) {
        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.isDirectory(base) || isSymlink(base)) {
            return "Error";
        }

        Path tempFile = null;
        Path target = base.resolve(TARGET_NAME).normalize();
        if (!target.startsWith(base)) {
            return "Error";
        }

        boolean posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
        Set<PosixFilePermission> restrictive = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
        FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(restrictive);

        try {
            // Create a temp file in base directory with restrictive permissions if POSIX
            if (posix) {
                tempFile = Files.createTempFile(base, ".secret_tmp_", ".tmp", attr);
            } else {
                tempFile = Files.createTempFile(base, ".secret_tmp_", ".tmp");
                File f = tempFile.toFile();
                f.setReadable(true, true);
                f.setWritable(true, true);
                f.setExecutable(false, true);
            }

            // Write content
            try (FileChannel ch = FileChannel.open(tempFile, StandardOpenOption.WRITE)) {
                ch.write(ByteBuffer.wrap(CONTENT.getBytes(StandardCharsets.UTF_8)));
                ch.force(true); // fsync file
            }

            // Ensure restrictive perms on temp file
            setRestrictivePerms(tempFile, posix, restrictive);

            // Move atomically into place; replace if exists
            try {
                Files.move(tempFile, target, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tempFile, target, StandardCopyOption.REPLACE_EXISTING);
            }

            // Ensure restrictive perms on final file
            setRestrictivePerms(target, posix, restrictive);

            // Print permissions
            return "Permissions: " + permString(target, posix);
        } catch (IOException e) {
            try {
                if (tempFile != null) Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {}
            return "Error";
        }
    }

    private static boolean isSymlink(Path p) {
        try {
            return Files.isSymbolicLink(p) || Files.readAttributes(p, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS).isSymbolicLink();
        } catch (IOException e) {
            return true;
        }
    }

    private static void setRestrictivePerms(Path p, boolean posix, Set<PosixFilePermission> restrictive) throws IOException {
        if (posix) {
            PosixFileAttributeView view = Files.getFileAttributeView(p, PosixFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);
            if (view != null) {
                view.setPermissions(restrictive);
            }
        } else {
            File f = p.toFile();
            // Best-effort on non-POSIX
            f.setReadable(true, true);
            f.setWritable(true, true);
            f.setExecutable(false, true);
        }
    }

    private static String permString(Path p, boolean posix) throws IOException {
        if (posix) {
            PosixFileAttributes a = Files.readAttributes(p, PosixFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            Set<PosixFilePermission> perms = a.permissions();
            StringBuilder sb = new StringBuilder(10);
            sb.append('-');
            sb.append(perms.contains(PosixFilePermission.OWNER_READ) ? 'r' : '-');
            sb.append(perms.contains(PosixFilePermission.OWNER_WRITE) ? 'w' : '-');
            sb.append(perms.contains(PosixFilePermission.OWNER_EXECUTE) ? 'x' : '-');
            sb.append(perms.contains(PosixFilePermission.GROUP_READ) ? 'r' : '-');
            sb.append(perms.contains(PosixFilePermission.GROUP_WRITE) ? 'w' : '-');
            sb.append(perms.contains(PosixFilePermission.GROUP_EXECUTE) ? 'x' : '-');
            sb.append(perms.contains(PosixFilePermission.OTHERS_READ) ? 'r' : '-');
            sb.append(perms.contains(PosixFilePermission.OTHERS_WRITE) ? 'w' : '-');
            sb.append(perms.contains(PosixFilePermission.OTHERS_EXECUTE) ? 'x' : '-');

            int mode = 0;
            mode |= perms.contains(PosixFilePermission.OWNER_READ) ? 0400 : 0;
            mode |= perms.contains(PosixFilePermission.OWNER_WRITE) ? 0200 : 0;
            mode |= perms.contains(PosixFilePermission.OWNER_EXECUTE) ? 0100 : 0;
            mode |= perms.contains(PosixFilePermission.GROUP_READ) ? 0040 : 0;
            mode |= perms.contains(PosixFilePermission.GROUP_WRITE) ? 0020 : 0;
            mode |= perms.contains(PosixFilePermission.GROUP_EXECUTE) ? 0010 : 0;
            mode |= perms.contains(PosixFilePermission.OTHERS_READ) ? 0004 : 0;
            mode |= perms.contains(PosixFilePermission.OTHERS_WRITE) ? 0002 : 0;
            mode |= perms.contains(PosixFilePermission.OTHERS_EXECUTE) ? 0001 : 0;
            return sb.toString() + " (" + String.format("%04o", mode) + ")";
        } else {
            File f = p.toFile();
            String ownerPerms = (f.canRead() ? "r" : "-") + (f.canWrite() ? "w" : "-") + (f.canExecute() ? "x" : "-");
            String s = "-" + ownerPerms + "------";
            // Approximate numeric as 0600 when we set it
            return s + " (0600)";
        }
    }
}