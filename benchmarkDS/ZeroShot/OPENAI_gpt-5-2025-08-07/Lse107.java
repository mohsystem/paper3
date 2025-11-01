import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileStore;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

public class Lse107 {

    public static String createAndRestrict(String filePath, String data) {
        Path path = Paths.get(filePath).normalize();

        try {
            if (Files.exists(path, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(path)) {
                throw new IOException("Refusing to operate on symlink: " + filePath);
            }

            boolean posixSupportedFs = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
            boolean posixOnStore = false;
            try {
                FileStore store = Files.getFileStore(path.getParent() != null ? path.getParent() : path);
                posixOnStore = store.supportsFileAttributeView(PosixFileAttributeView.class);
            } catch (Exception ignored) {
                // If we cannot determine, fallback later.
            }
            boolean canUsePosix = posixSupportedFs && posixOnStore;

            if (!Files.exists(path, LinkOption.NOFOLLOW_LINKS)) {
                if (canUsePosix) {
                    Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                    Files.createFile(path);
                    Files.setPosixFilePermissions(path, perms);
                } else {
                    Files.createFile(path);
                    secureFallbackPermissions(path.toFile());
                }
            } else {
                // File exists; ensure it's not a directory
                if (Files.isDirectory(path, LinkOption.NOFOLLOW_LINKS)) {
                    throw new IOException("Path is a directory: " + filePath);
                }
            }

            try (OutputStream os = Files.newOutputStream(path, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                os.write(data.getBytes(StandardCharsets.UTF_8));
                os.flush();
            }

            // Enforce restrictive permissions after write as well
            if (canUsePosix) {
                Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
            } else {
                secureFallbackPermissions(path.toFile());
            }

            String permStr = getPermissionString(path);
            System.out.println(permStr);
            return permStr;
        } catch (IOException e) {
            String msg = "ERROR: " + e.getMessage();
            System.out.println(msg);
            return msg;
        }
    }

    private static void secureFallbackPermissions(File f) throws IOException {
        // Best-effort on non-POSIX: owner-only read/write, no exec for anyone
        if (!f.setReadable(false, false)) { /* ignore */ }
        if (!f.setWritable(false, false)) { /* ignore */ }
        if (!f.setExecutable(false, false)) { /* ignore */ }

        if (!f.setReadable(true, true)) { /* ignore */ }
        if (!f.setWritable(true, true)) { /* ignore */ }
        if (!f.setExecutable(false, true)) { /* ensure not executable by owner */ }
    }

    private static String getPermissionString(Path path) {
        try {
            FileStore store = Files.getFileStore(path.getParent() != null ? path.getParent() : path);
            boolean canUsePosix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix")
                    && store.supportsFileAttributeView(PosixFileAttributeView.class);
            if (canUsePosix) {
                Set<PosixFilePermission> p = Files.getPosixFilePermissions(path, LinkOption.NOFOLLOW_LINKS);
                StringBuilder sb = new StringBuilder();
                sb.append('-');
                sb.append(p.contains(PosixFilePermission.OWNER_READ) ? 'r' : '-');
                sb.append(p.contains(PosixFilePermission.OWNER_WRITE) ? 'w' : '-');
                sb.append(p.contains(PosixFilePermission.OWNER_EXECUTE) ? 'x' : '-');
                sb.append(p.contains(PosixFilePermission.GROUP_READ) ? 'r' : '-');
                sb.append(p.contains(PosixFilePermission.GROUP_WRITE) ? 'w' : '-');
                sb.append(p.contains(PosixFilePermission.GROUP_EXECUTE) ? 'x' : '-');
                sb.append(p.contains(PosixFilePermission.OTHERS_READ) ? 'r' : '-');
                sb.append(p.contains(PosixFilePermission.OTHERS_WRITE) ? 'w' : '-');
                sb.append(p.contains(PosixFilePermission.OTHERS_EXECUTE) ? 'x' : '-');
                return sb.toString();
            }
        } catch (Exception ignored) {
        }
        // Fallback approximation
        File f = path.toFile();
        StringBuilder sb = new StringBuilder();
        sb.append('-');
        sb.append(f.canRead() ? 'r' : '-');
        sb.append(f.canWrite() ? 'w' : '-');
        sb.append(f.canExecute() ? 'x' : '-');
        // Assume others none (owner-only intent)
        sb.append("------");
        return sb.toString();
    }

    public static void main(String[] args) {
        String secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        createAndRestrict("secret.txt", secret);
        createAndRestrict("secret1.txt", secret);
        createAndRestrict("secret2.txt", secret);
        createAndRestrict("secret3.txt", secret);
        createAndRestrict("secret4.txt", secret);
    }
}