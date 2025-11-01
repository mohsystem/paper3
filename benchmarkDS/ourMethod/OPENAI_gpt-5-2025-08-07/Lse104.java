import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;

public class Lse104 {
    private static final int MAX_PATH_LEN = 4096;
    private static final String CONTENT = "important_config";

    public static boolean secureWriteImportantConfig(String baseDirStr, String relativePathStr) {
        if (baseDirStr == null || relativePathStr == null) return false;
        if (baseDirStr.length() == 0 || relativePathStr.length() == 0) return false;
        if (baseDirStr.indexOf('\0') >= 0 || relativePathStr.indexOf('\0') >= 0) return false;
        if (relativePathStr.length() > MAX_PATH_LEN) return false;
        // Reject absolute and dangerous segments
        if (relativePathStr.startsWith("/") || relativePathStr.startsWith("\\")) return false;
        String[] parts = relativePathStr.split("/");
        for (String p : parts) {
            if (p.isEmpty() || p.equals(".") || p.equals("..")) return false;
        }

        try {
            Path base = Paths.get(baseDirStr).toAbsolutePath().normalize();
            // Require base to exist and be directory without being a symlink
            if (!Files.exists(base, LinkOption.NOFOLLOW_LINKS) || !Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) return false;
            if (Files.isSymbolicLink(base)) return false;

            Path target = base.resolve(relativePathStr).normalize();
            // Ensure target stays within base
            if (!target.startsWith(base)) return false;

            Path parent = target.getParent();
            if (parent == null) return false;

            // Ensure all parent components exist, are directories, and not symlinks
            Path cur = base;
            for (Path comp : base.relativize(parent)) {
                cur = cur.resolve(comp);
                if (!Files.exists(cur, LinkOption.NOFOLLOW_LINKS)) return false;
                if (Files.isSymbolicLink(cur)) return false;
                if (!Files.isDirectory(cur, LinkOption.NOFOLLOW_LINKS)) return false;
            }

            // If target exists and is a symlink, refuse
            if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
                return false;
            }

            // Create a temp file in the same directory
            String safeName = target.getFileName().toString();
            if (safeName.length() == 0 || safeName.length() > 255) return false;

            SecureRandom sr = new SecureRandom();
            byte[] rnd = new byte[8];
            sr.nextBytes(rnd);
            StringBuilder sb = new StringBuilder();
            for (byte b : rnd) sb.append(String.format("%02x", b));
            String tmpName = ".tmp-" + sb + "-" + System.nanoTime();
            Path tmp = parent.resolve(tmpName);

            // Create the temp file with restrictive perms and no follow symlinks
            // Java does not expose O_NOFOLLOW for creation, but we ensured parent has no symlinks.
            try (FileChannel fc = FileChannel.open(tmp,
                    StandardOpenOption.WRITE,
                    StandardOpenOption.CREATE_NEW)) {
                // Set permissions to 0600
                try {
                    Files.setPosixFilePermissions(tmp, PosixFilePermissions.fromString("rw-------"));
                } catch (UnsupportedOperationException ignored) {
                    // Non-POSIX FS: best effort
                }

                ByteBuffer buf = ByteBuffer.wrap(CONTENT.getBytes(StandardCharsets.UTF_8));
                while (buf.hasRemaining()) {
                    fc.write(buf);
                }
                fc.force(true);
            } catch (IOException e) {
                // Clean up temp if partially created
                try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
                return false;
            }

            // Atomically move temp to target, replacing if exists
            try {
                // Ensure final parent is still safe
                if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
                    Files.deleteIfExists(tmp);
                    return false;
                }
                Files.move(tmp, target, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                // Fallback to regular replace in same dir
                try {
                    Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING);
                } catch (IOException ex) {
                    try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
                    return false;
                }
            } catch (IOException e) {
                try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
                return false;
            }

            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private static void prepareDir(Path base, String sub) throws IOException {
        Path p = base.resolve(sub).normalize();
        if (!p.startsWith(base)) throw new IOException("invalid");
        Files.createDirectories(p);
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("safe-base-");
            // Prepare subdir for tests
            prepareDir(base, "subdir");

            List<String> tests = new ArrayList<>();
            tests.add("config.txt");            // expected: true
            tests.add("subdir/settings.conf");  // expected: true
            tests.add("../escape.txt");         // expected: false
            tests.add("/abs/path.txt");         // expected: false
            tests.add("nested/../final.conf");  // expected: false (reject '..')

            for (String rel : tests) {
                boolean ok = secureWriteImportantConfig(base.toString(), rel);
                System.out.println(rel + " -> " + (ok ? "OK" : "FAIL"));
            }
        } catch (IOException e) {
            System.out.println("Setup failed");
        }
    }
}