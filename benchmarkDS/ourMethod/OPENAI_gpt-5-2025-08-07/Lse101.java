import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse101 {

    public static boolean writeImportantConfig(Path baseDir) {
        if (baseDir == null) {
            return false;
        }
        try {
            Path realBase;
            try {
                realBase = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                return false;
            }

            BasicFileAttributes attrs;
            try {
                attrs = Files.readAttributes(realBase, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                return false;
            }
            if (!attrs.isDirectory()) {
                return false;
            }

            Path dest = realBase.resolve("important_config");
            // Create a secure temp file within the base directory
            Path tmp;
            Set<PosixFilePermission> perms600 = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            try {
                tmp = Files.createTempFile(realBase, "tmp.important_config.", ".tmp",
                        PosixFilePermissions.asFileAttribute(perms600));
            } catch (UnsupportedOperationException uoe) {
                try {
                    tmp = Files.createTempFile(realBase, "tmp.important_config.", ".tmp");
                } catch (IOException e) {
                    return false;
                }
                try {
                    Files.setPosixFilePermissions(tmp, perms600);
                } catch (Exception ignore) {
                    // Best effort on non-POSIX
                }
            } catch (IOException e) {
                return false;
            }

            boolean ok = false;
            try (FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE)) {
                byte[] data = "important_config".getBytes(StandardCharsets.UTF_8);
                ByteBuffer buf = ByteBuffer.wrap(data);
                while (buf.hasRemaining()) {
                    int w = ch.write(buf);
                    if (w < 0) {
                        return false;
                    }
                }
                ch.force(true);
                try {
                    Files.move(tmp, dest,
                            StandardCopyOption.ATOMIC_MOVE,
                            StandardCopyOption.REPLACE_EXISTING);
                } catch (AtomicMoveNotSupportedException am) {
                    // Fallback to non-atomic replace if ATOMIC_MOVE is not supported
                    try {
                        Files.move(tmp, dest, StandardCopyOption.REPLACE_EXISTING);
                    } catch (IOException e) {
                        return false;
                    }
                }
                try {
                    Files.setPosixFilePermissions(dest, perms600);
                } catch (Exception ignore) {
                    // Best effort on non-POSIX
                }
                ok = true;
            } catch (IOException e) {
                ok = false;
            } finally {
                try {
                    if (!ok && Files.exists(tmp)) {
                        Files.deleteIfExists(tmp);
                    }
                } catch (IOException ignore) {
                }
            }
            return ok;
        } catch (Exception ex) {
            return false;
        }
    }

    public static void main(String[] args) {
        Path[] tests = new Path[] {
                Paths.get("."),
                Paths.get("java_out1"),
                Paths.get("java_out1/subdir"),
                Paths.get("java out 2"),
                Paths.get("java_out3")
        };
        for (Path p : tests) {
            try {
                Files.createDirectories(p);
            } catch (IOException ignore) {
            }
            boolean res = writeImportantConfig(p);
            System.out.println((p.toString().isEmpty() ? "." : p.toString()) + ": " + (res ? "ok" : "fail"));
        }
    }
}