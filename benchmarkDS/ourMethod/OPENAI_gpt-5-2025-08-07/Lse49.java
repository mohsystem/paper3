import java.io.BufferedInputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.CopyOption;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

public class Lse49 {

    private static final int TAR_BLOCK = 512;
    private static final long MAX_FILE_SIZE = 1_000_000_000L; // 1GB limit

    private static boolean isZeroBlock(byte[] b) {
        for (byte value : b) {
            if (value != 0) return false;
        }
        return true;
    }

    private static String readString(byte[] buf, int off, int len) {
        int end = off + len;
        int i = off;
        for (; i < end; i++) {
            if (buf[i] == 0) break;
        }
        return new String(buf, off, i - off, StandardCharsets.US_ASCII);
    }

    private static long parseOctal(byte[] buf, int off, int len) throws IOException {
        long val = 0;
        int start = off;
        int end = off + len;
        // skip leading spaces and nulls
        while (start < end && (buf[start] == 0 || buf[start] == ' ')) start++;
        for (int i = start; i < end; i++) {
            byte c = buf[i];
            if (c == 0 || c == ' ' || c == 0x00) break;
            if (c < '0' || c > '7') break;
            long digit = c - '0';
            if (val > (Long.MAX_VALUE >> 3)) throw new IOException("size overflow");
            val = (val << 3) + digit;
        }
        return val;
    }

    private static boolean isSafeRelativePath(String p) {
        if (p == null || p.isEmpty()) return false;
        if (p.charAt(0) == '/') return false;
        String[] parts = p.split("/");
        for (String seg : parts) {
            if (seg.equals("") || seg.equals(".") || seg.equals("..")) return false;
        }
        return true;
    }

    private static void ensureDirSecure(Path base, Path dir) throws IOException {
        Path normBase = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path current = normBase;
        for (Path part : normBase.relativize(base.resolve(dir)).normalize()) {
            if (part.getNameCount() == 0) continue;
            current = current.resolve(part.toString());
            // Do not follow symlinks; check before creating
            if (Files.exists(current, LinkOption.NOFOLLOW_LINKS)) {
                if (Files.isSymbolicLink(current)) {
                    throw new IOException("symlink encountered in path: " + current);
                }
                if (Files.isDirectory(current, LinkOption.NOFOLLOW_LINKS)) {
                    continue;
                } else {
                    throw new IOException("non-directory in path: " + current);
                }
            } else {
                Files.createDirectory(current);
                try {
                    // set restrictive perms if POSIX
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE,
                            PosixFilePermission.OWNER_EXECUTE
                    );
                    Files.setPosixFilePermissions(current, perms);
                } catch (UnsupportedOperationException ignore) {
                }
            }
        }
    }

    private static void writeRegularFileSecure(Path base, String relPath, InputStream src, long size) throws IOException {
        if (size < 0 || size > MAX_FILE_SIZE) throw new IOException("file too large");
        Path target = base.resolve(relPath).normalize();
        Path normBase = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
        if (!target.startsWith(normBase)) throw new IOException("path escapes base");
        Path parent = target.getParent();
        if (parent != null) {
            ensureDirSecure(normBase, normBase.relativize(parent));
        }
        // Refuse if any parent is symlink
        Path cur = normBase;
        for (Path part : normBase.relativize(target)) {
            cur = cur.resolve(part.toString());
            if (Files.isSymbolicLink(cur)) {
                throw new IOException("symlink in path");
            }
        }
        // Create new file exclusively
        try (SeekableByteChannel ch = Files.newByteChannel(target,
                EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE))) {
            byte[] buf = new byte[8192];
            long remaining = size;
            while (remaining > 0) {
                int toRead = (int) Math.min(buf.length, remaining);
                int n = src.read(buf, 0, toRead);
                if (n < 0) throw new EOFException("unexpected EOF in file data");
                ch.write(ByteBuffer.wrap(buf, 0, n));
                remaining -= n;
            }
            try {
                ch.force(true);
            } catch (IOException ignore) {
            }
        } catch (FileAlreadyExistsException e) {
            throw new IOException("file exists");
        }
        // set restrictive perms if POSIX
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.setPosixFilePermissions(target, perms);
        } catch (UnsupportedOperationException ignore) {
        }
    }

    public static void extractTarSecure(Path archivePath, Path destDir) throws IOException {
        if (archivePath == null || destDir == null) throw new IOException("invalid args");
        if (Files.isSymbolicLink(archivePath)) throw new IOException("refuse symlink archive");
        if (!Files.exists(archivePath, LinkOption.NOFOLLOW_LINKS)) throw new IOException("archive not found");
        if (!Files.isRegularFile(archivePath, LinkOption.NOFOLLOW_LINKS)) throw new IOException("not a regular file");
        if (!Files.exists(destDir)) {
            Files.createDirectories(destDir);
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(destDir, perms);
            } catch (UnsupportedOperationException ignore) {
            }
        }
        Path normBase = destDir.toAbsolutePath().normalize();

        try (FileChannel fc = FileChannel.open(archivePath, StandardOpenOption.READ);
             InputStream raw = Channels.newInputStream(fc);
             BufferedInputStream in = new BufferedInputStream(raw, 64 * 1024)) {

            byte[] header = new byte[TAR_BLOCK];
            while (true) {
                int r = in.readNBytes(header, 0, TAR_BLOCK);
                if (r == 0) break;
                if (r < TAR_BLOCK) throw new EOFException("truncated header");
                if (isZeroBlock(header)) {
                    // possibly second zero block
                    int r2 = in.readNBytes(header, 0, TAR_BLOCK);
                    if (r2 < TAR_BLOCK || isZeroBlock(header)) break;
                    else throw new IOException("invalid tar end");
                }
                String name = readString(header, 0, 100);
                String prefix = readString(header, 345, 155);
                String pathInTar = prefix.isEmpty() ? name : (prefix + "/" + name);
                if (!isSafeRelativePath(pathInTar)) {
                    // skip this entry safely
                    long sz = parseOctal(header, 124, 12);
                    long skip = sz + ((TAR_BLOCK - (sz % TAR_BLOCK)) % TAR_BLOCK);
                    long skipped = in.skipNBytes(skip);
                    if (skipped != skip) throw new EOFException("truncated skip");
                    continue;
                }
                byte typeflag = header[156];
                long size = parseOctal(header, 124, 12);
                if (size < 0 || size > MAX_FILE_SIZE) {
                    long skip = size < 0 ? 0 : size + ((TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK);
                    if (skip > 0) in.skipNBytes(skip);
                    throw new IOException("entry too large");
                }
                if (typeflag == '5') {
                    Path dirTarget = normBase.resolve(pathInTar).normalize();
                    if (!dirTarget.startsWith(normBase)) throw new IOException("path escapes base");
                    ensureDirSecure(normBase, normBase.relativize(dirTarget));
                } else if (typeflag == '0' || typeflag == 0) {
                    writeRegularFileSecure(normBase, pathInTar, in, size);
                } else {
                    // skip non-regular entries securely
                    long skip = size + ((TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK);
                    if (skip > 0) in.skipNBytes(skip);
                }
                long pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
                if (pad > 0 && (typeflag == '0' || typeflag == 0)) {
                    long skipped = in.skipNBytes(pad);
                    if (skipped != pad) throw new EOFException("truncated padding");
                }
            }
        }
    }

    public static void main(String[] args) {
        // 5 test cases: paths may or may not exist; errors are printed.
        Path dest = Paths.get("/tmp/unpack");
        String[] archives = new String[] {
                "archive.zip",
                "./archive.zip",
                "/does/not/exist/archive.zip",
                "archive.zip",
                "archive.zip"
        };
        for (int i = 0; i < archives.length; i++) {
            try {
                extractTarSecure(Paths.get(archives[i]), dest);
                System.out.println("Test " + (i+1) + ": OK");
            } catch (Exception e) {
                System.out.println("Test " + (i+1) + ": " + e.getMessage());
            }
        }
    }
}