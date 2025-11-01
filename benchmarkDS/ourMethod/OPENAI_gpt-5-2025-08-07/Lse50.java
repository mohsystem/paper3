import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;

public class Lse50 {

    private static final int TAR_BLOCK = 512;
    private static final long MAX_ENTRY_SIZE = 100L * 1024L * 1024L; // 100MB cap per entry

    public static boolean extractTar(String archivePath, String destDir) {
        if (archivePath == null || destDir == null) {
            System.err.println("Invalid input");
            return false;
        }
        if (archivePath.length() > 4096 || destDir.length() > 4096) {
            System.err.println("Path too long");
            return false;
        }
        Path dest = Paths.get(destDir).toAbsolutePath().normalize();
        try {
            if (Files.exists(dest, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(dest)) {
                System.err.println("Destination is a symlink");
                return false;
            }
            if (!Files.exists(dest, LinkOption.NOFOLLOW_LINKS)) {
                Files.createDirectories(dest);
                try {
                    // Set restrictive permissions if POSIX
                    Files.setPosixFilePermissions(dest, EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE, PosixFilePermission.OWNER_EXECUTE));
                } catch (UnsupportedOperationException ignored) {
                }
            }
        } catch (IOException e) {
            System.err.println("Failed to prepare destination");
            return false;
        }

        // Best-effort check to avoid following symlinks for source
        try {
            Path src = Paths.get(archivePath);
            if (Files.isSymbolicLink(src)) {
                System.err.println("Source is a symlink");
                return false;
            }
        } catch (Exception ignored) {
        }

        try (FileInputStream fis = new FileInputStream(archivePath);
             BufferedInputStream bis = new BufferedInputStream(fis)) {

            byte[] header = new byte[TAR_BLOCK];
            boolean sawZeroBlock = false;

            while (true) {
                int read = readFully(bis, header, 0, TAR_BLOCK);
                if (read == -1) {
                    // Unexpected EOF
                    break;
                }
                if (read != TAR_BLOCK) {
                    System.err.println("Truncated header");
                    return false;
                }

                if (isAllZero(header)) {
                    if (sawZeroBlock) {
                        // Two consecutive zero blocks: EOF
                        break;
                    } else {
                        sawZeroBlock = true;
                        continue;
                    }
                } else {
                    sawZeroBlock = false;
                }

                if (!validChecksum(header)) {
                    System.err.println("Invalid header checksum");
                    return false;
                }

                String name = extractString(header, 0, 100);
                String prefix = extractString(header, 345, 155);
                String relPath = (prefix.isEmpty() ? name : (prefix + "/" + name));
                char typeFlag = (char) header[156];
                long size = parseOctal(header, 124, 12);
                if (size < 0 || size > MAX_ENTRY_SIZE) {
                    System.err.println("Entry size invalid");
                    return false;
                }

                if (!isSafeRelativePath(relPath)) {
                    System.err.println("Unsafe path in archive: " + relPath);
                    return false;
                }

                Path target = dest.resolve(relPath).normalize();
                if (!target.startsWith(dest)) {
                    System.err.println("Path escapes destination");
                    return false;
                }

                switch (typeFlag) {
                    case '0':
                    case 0: // old tar uses NUL
                        // Regular file
                        if (!createParentsSafely(dest, target.getParent())) {
                            System.err.println("Failed to create parent directories");
                            return false;
                        }
                        if (!Files.exists(target.getParent(), LinkOption.NOFOLLOW_LINKS)) {
                            System.err.println("Parent directory missing");
                            return false;
                        }
                        try (OutputStream os = Files.newOutputStream(target, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                            if (!copyLimited(bis, os, size)) {
                                System.err.println("Truncated file data");
                                return false;
                            }
                        } catch (FileAlreadyExistsException e) {
                            System.err.println("Refusing to overwrite existing file: " + target);
                            return false;
                        }
                        // Skip padding
                        skipPadding(bis, size);
                        break;
                    case '5':
                        // Directory
                        if (!createParentsSafely(dest, target)) {
                            System.err.println("Failed to create directory");
                            return false;
                        }
                        // Skip padding for directories (size should be zero, but still pad)
                        skipPadding(bis, size);
                        break;
                    default:
                        // Unsupported entry types; skip payload safely
                        if (!skipLimited(bis, size)) {
                            System.err.println("Truncated data for skipped entry");
                            return false;
                        }
                        skipPadding(bis, size);
                        break;
                }
            }

            return true;
        } catch (IOException e) {
            System.err.println("IO error");
            return false;
        }
    }

    private static boolean isAllZero(byte[] buf) {
        for (byte b : buf) {
            if (b != 0) return false;
        }
        return true;
    }

    private static String extractString(byte[] buf, int off, int len) {
        int end = off + len;
        int i = off;
        while (i < end && buf[i] != 0) i++;
        return new String(buf, off, i - off, StandardCharsets.US_ASCII).trim();
    }

    private static long parseOctal(byte[] buf, int off, int len) {
        long val = 0;
        int i = off;
        // Skip leading spaces and nulls
        while (i < off + len && (buf[i] == 0 || buf[i] == ' ')) i++;
        for (; i < off + len; i++) {
            byte b = buf[i];
            if (b == 0 || b == ' ') break;
            if (b < '0' || b > '7') return -1;
            val = (val << 3) + (b - '0');
            if (val < 0) return -1;
        }
        return val;
    }

    private static boolean validChecksum(byte[] header) {
        long stored = parseOctal(header, 148, 8);
        if (stored < 0) return false;
        long sum = 0;
        for (int i = 0; i < TAR_BLOCK; i++) {
            if (i >= 148 && i < 156) {
                sum += 32; // space
            } else {
                sum += (header[i] & 0xFF);
            }
        }
        return sum == stored;
    }

    private static boolean isSafeRelativePath(String p) {
        if (p == null || p.isEmpty()) return false;
        if (p.startsWith("/")) return false;
        if (p.contains("..")) return false;
        if (p.contains("\\"))
            return false;
        // Also reject absolute Windows-like; not relevant on UNIX but safe
        if (p.matches("^[A-Za-z]:.*")) return false;
        return true;
    }

    private static boolean createParentsSafely(Path base, Path targetDir) {
        if (targetDir == null) return true;
        try {
            Path norm = targetDir.normalize();
            if (!norm.startsWith(base)) return false;
            // Walk components and ensure no symlinks
            Path cur = base;
            for (Path part : base.relativize(norm)) {
                cur = cur.resolve(part);
                if (Files.exists(cur, LinkOption.NOFOLLOW_LINKS)) {
                    if (Files.isSymbolicLink(cur)) return false;
                    if (Files.isDirectory(cur, LinkOption.NOFOLLOW_LINKS)) continue;
                    return false;
                } else {
                    Files.createDirectory(cur);
                    try {
                        Files.setPosixFilePermissions(cur, EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE, PosixFilePermission.OWNER_EXECUTE));
                    } catch (UnsupportedOperationException ignored) {
                    }
                }
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    private static int readFully(InputStream in, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int r = in.read(buf, off + total, len - total);
            if (r < 0) {
                return total == 0 ? -1 : total;
            }
            total += r;
        }
        return total;
    }

    private static boolean copyLimited(InputStream in, OutputStream out, long bytes) throws IOException {
        byte[] buf = new byte[32 * 1024];
        long remaining = bytes;
        while (remaining > 0) {
            int toRead = (int) Math.min(buf.length, remaining);
            int r = in.read(buf, 0, toRead);
            if (r < 0) return false;
            out.write(buf, 0, r);
            remaining -= r;
        }
        out.flush();
        return true;
    }

    private static boolean skipLimited(InputStream in, long bytes) throws IOException {
        long remaining = bytes;
        byte[] buf = new byte[8192];
        while (remaining > 0) {
            int toRead = (int) Math.min(buf.length, remaining);
            int r = in.read(buf, 0, toRead);
            if (r < 0) return false;
            remaining -= r;
        }
        return true;
    }

    private static void skipPadding(InputStream in, long size) throws IOException {
        long pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
        if (pad == 0) return;
        long remaining = pad;
        byte[] tmp = new byte[512];
        while (remaining > 0) {
            int r = in.read(tmp, 0, (int) Math.min(tmp.length, remaining));
            if (r < 0) break;
            remaining -= r;
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Test1: " + extractTar("archive.zip", "/tmp/unpack"));
        System.out.println("Test2: " + extractTar("nonexistent.tar", "/tmp/unpack"));
        System.out.println("Test3: " + extractTar("", "/tmp/unpack"));
        System.out.println("Test4: " + extractTar("archive.zip", "/tmp/unpack/sub/.."));
        System.out.println("Test5: " + extractTar("archive.zip", "/tmp/unpack")); // repeated to check idempotent failure on overwriting
    }
}