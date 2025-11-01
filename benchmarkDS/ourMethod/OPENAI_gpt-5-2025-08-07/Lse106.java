import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFileAttributeView;
import java.security.SecureRandom;
import java.util.EnumSet;
import java.util.Objects;
import java.util.Set;

public class Lse106 {

    // Create a string with some super secret info, write it to a securely created file
    // under baseDir with the given filename, set owner read-only, and return the path string.
    public static String writeSecretFile(Path baseDir, String fileName) throws IOException {
        Objects.requireNonNull(baseDir, "baseDir");
        Objects.requireNonNull(fileName, "fileName");

        // Validate filename: restrict to safe characters, no separators or traversal.
        if (fileName.isEmpty() || fileName.length() > 255) {
            throw new IOException("Invalid filename length");
        }
        for (int i = 0; i < fileName.length(); i++) {
            char c = fileName.charAt(i);
            if (!(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') &&
                c != '.' && c != '-' && c != '_' ) {
                throw new IOException("Invalid filename characters");
            }
        }
        if (fileName.contains("/") || fileName.contains("\\") || fileName.equals(".") || fileName.equals("..")) {
            throw new IOException("Invalid filename path components");
        }

        // Resolve and ensure base directory is real and not a symlink
        Path baseReal = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        if (!Files.isDirectory(baseReal, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base is not a directory");
        }

        // Create a random temp filename in the base directory
        SecureRandom rng = new SecureRandom();
        byte[] rnd = new byte[12];
        rng.nextBytes(rnd);
        StringBuilder hex = new StringBuilder();
        for (byte b : rnd) {
            hex.append(String.format("%02x", b));
        }
        String tmpName = ".tmp_" + hex + ".part";
        Path tmpPath = baseReal.resolve(tmpName).normalize();
        if (!tmpPath.getParent().equals(baseReal)) {
            throw new IOException("Temp path escaped base");
        }

        // Prepare permissions
        boolean posix = Files.getFileStore(baseReal).supportsFileAttributeView(PosixFileAttributeView.class);
        FileAttribute<Set<PosixFilePermission>> attr0600 = null;
        if (posix) {
            attr0600 = new FileAttribute<Set<PosixFilePermission>>() {
                @Override
                public String name() { return "posix:permissions"; }
                @Override
                public Set<PosixFilePermission> value() {
                    return EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                }
            };
        }

        // Create the temp file with restrictive perms, write, fsync, set read-only, then atomic move
        String secret = "TOP-SECRET: Highly confidential data. Do not disclose.";
        Path target = baseReal.resolve(fileName).normalize();
        if (!target.getParent().equals(baseReal)) {
            throw new IOException("Target path escaped base");
        }

        // Create temp file (CREATE_NEW semantics)
        if (Files.exists(tmpPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Temp already exists");
        }
        if (posix) {
            Files.createFile(tmpPath, attr0600);
        } else {
            Files.createFile(tmpPath);
            // Best effort: restrict with basic API on non-POSIX
            tmpPath.toFile().setReadable(true, true);
            tmpPath.toFile().setWritable(true, true);
            tmpPath.toFile().setExecutable(false, false);
        }

        // Open and write using a channel, then force to disk
        try (FileChannel ch = FileChannel.open(tmpPath, StandardOpenOption.WRITE)) {
            ByteBuffer buf = StandardCharsets.UTF_8.encode(secret);
            while (buf.hasRemaining()) {
                ch.write(buf);
            }
            ch.force(true);
        }

        // Set read-only for owner on temp file
        if (posix) {
            Set<PosixFilePermission> ro = EnumSet.of(PosixFilePermission.OWNER_READ);
            Files.setPosixFilePermissions(tmpPath, ro);
        } else {
            tmpPath.toFile().setReadable(true, true);
            tmpPath.toFile().setWritable(false, false);
            tmpPath.toFile().setExecutable(false, false);
        }

        // Ensure target does not exist; then atomic move into place
        try {
            Files.move(tmpPath, target, StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            // Fallback to regular move (rename) within same directory
            Files.move(tmpPath, target, StandardCopyOption.REPLACE_EXISTING);
        }

        return target.toString();
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("safe_base_");
            // Five test cases
            String[] names = new String[] {
                "secret1.txt",
                "secret2.txt",
                "alpha-3.log",
                "note_4.dat",
                "report5"
            };
            for (String n : names) {
                String p = writeSecretFile(base, n);
                System.out.println(p);
            }
        } catch (Exception e) {
            // Print only a generic message to avoid leaking details
            System.out.println("Operation failed");
        }
    }
}