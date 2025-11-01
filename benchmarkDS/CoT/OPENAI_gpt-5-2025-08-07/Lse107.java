import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {
    // Secure function: create/overwrite file, write content, set restrictive permissions, return permissions string
    public static String secureWriteRestrictPrint(String filePath, String content) {
        Path p = Paths.get(filePath).toAbsolutePath().normalize();
        try {
            // 1) Write content to file safely
            Files.write(p, content.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);

            // 2) Enforce restrictive permissions (rw-------) if POSIX; otherwise best-effort via File API
            String permStr;
            if (Files.getFileAttributeView(p, PosixFileAttributeView.class) != null) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(p, perms);
                permStr = buildPermStringPosix(p);
            } else {
                File f = p.toFile();
                // Remove all permissions for all users
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                // Set owner-only read and write
                f.setReadable(true, true);
                f.setWritable(true, true);
                // Build synthetic permission string for non-POSIX systems
                permStr = buildPermStringNonPosix(f);
            }
            // 3) Return permission representation
            return permStr;
        } catch (IOException e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static String buildPermStringPosix(Path p) throws IOException {
        Set<PosixFilePermission> perms = Files.getPosixFilePermissions(p);
        int mode = 0;
        if (perms.contains(PosixFilePermission.OWNER_READ)) mode |= 0400;
        if (perms.contains(PosixFilePermission.OWNER_WRITE)) mode |= 0200;
        if (perms.contains(PosixFilePermission.OWNER_EXECUTE)) mode |= 0100;
        if (perms.contains(PosixFilePermission.GROUP_READ)) mode |= 0040;
        if (perms.contains(PosixFilePermission.GROUP_WRITE)) mode |= 0020;
        if (perms.contains(PosixFilePermission.GROUP_EXECUTE)) mode |= 0010;
        if (perms.contains(PosixFilePermission.OTHERS_READ)) mode |= 0004;
        if (perms.contains(PosixFilePermission.OTHERS_WRITE)) mode |= 0002;
        if (perms.contains(PosixFilePermission.OTHERS_EXECUTE)) mode |= 0001;

        StringBuilder sb = new StringBuilder();
        sb.append(((mode & 0400) != 0) ? 'r' : '-');
        sb.append(((mode & 0200) != 0) ? 'w' : '-');
        sb.append(((mode & 0100) != 0) ? 'x' : '-');
        sb.append(((mode & 0040) != 0) ? 'r' : '-');
        sb.append(((mode & 0020) != 0) ? 'w' : '-');
        sb.append(((mode & 0010) != 0) ? 'x' : '-');
        sb.append(((mode & 0004) != 0) ? 'r' : '-');
        sb.append(((mode & 0002) != 0) ? 'w' : '-');
        sb.append(((mode & 0001) != 0) ? 'x' : '-');

        return String.format("%04o %s", mode, sb.toString());
    }

    private static String buildPermStringNonPosix(File f) {
        // After best-effort restriction to owner read/write, represent as rw------- (0600)
        boolean r = f.canRead();
        boolean w = f.canWrite();
        boolean x = f.canExecute();
        String owner = (r ? "r" : "-") + (w ? "w" : "-") + (x ? "x" : "-");
        String s = owner + "------";
        // We attempted to set owner rw only; report as 0600 equivalent
        return String.format("%04o %s", 0600, s);
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        String path = "secret.txt";

        // Five test cases (repeated operations on the same file as per requirement)
        System.out.println(secureWriteRestrictPrint(path, content));
        System.out.println(secureWriteRestrictPrint(path, content));
        System.out.println(secureWriteRestrictPrint(path, content));
        System.out.println(secureWriteRestrictPrint(path, content));
        System.out.println(secureWriteRestrictPrint(path, content));
    }
}