import java.io.IOException;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse103 {
    public static boolean secureOpenWrite(String pathStr, boolean create, boolean trunc, boolean wrOnly, boolean excl) {
        try {
            if (excl && !create) {
                return false;
            }

            Path path = Paths.get(pathStr).normalize();

            if (Files.exists(path, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(path)) {
                return false;
            }

            Path parent = path.getParent();
            if (parent != null && !Files.exists(parent)) {
                try {
                    if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                        Files.createDirectories(parent, PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rwx------")));
                    } else {
                        Files.createDirectories(parent);
                    }
                } catch (FileAlreadyExistsException ignored) {
                }
            }

            Set<OpenOption> opts = new HashSet<>();
            opts.add(StandardOpenOption.WRITE);
            if (!wrOnly) {
                opts.add(StandardOpenOption.READ);
            }

            if (create && excl) {
                opts.add(StandardOpenOption.CREATE_NEW);
            } else if (create) {
                opts.add(StandardOpenOption.CREATE);
            }

            if (trunc && !(create && excl)) {
                opts.add(StandardOpenOption.TRUNCATE_EXISTING);
            }

            boolean posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
            if (posix && create) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                try (SeekableByteChannel ch = Files.newByteChannel(path, opts, attr)) {
                    // opened successfully
                }
            } else {
                try (SeekableByteChannel ch = Files.newByteChannel(path, opts)) {
                    // opened successfully
                }
            }

            if (Files.isSymbolicLink(path)) {
                return false;
            }

            return true;
        } catch (IOException | SecurityException | UnsupportedOperationException e) {
            return false;
        }
    }

    public static void main(String[] args) {
        try {
            Path dir = Paths.get("lse103_tmp_java");
            if (!Files.exists(dir)) {
                try {
                    if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                        Files.createDirectory(dir, PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rwx------")));
                    } else {
                        Files.createDirectory(dir);
                    }
                } catch (FileAlreadyExistsException ignored) {
                } catch (IOException e) {
                    System.out.println("false");
                    System.out.println("false");
                    System.out.println("false");
                    System.out.println("false");
                    System.out.println("false");
                    return;
                }
            }

            String f1 = dir.resolve("a.txt").toString();
            String f2 = dir.resolve("nonexist.txt").toString();

            System.out.println(secureOpenWrite(f1, true, false, true, true));
            System.out.println(secureOpenWrite(f1, true, false, true, true));
            System.out.println(secureOpenWrite(f1, false, true, true, false));
            System.out.println(secureOpenWrite(f1, false, false, true, false));
            System.out.println(secureOpenWrite(f2, false, false, true, false));
        } catch (Exception e) {
            System.out.println("false");
            System.out.println("false");
            System.out.println("false");
            System.out.println("false");
            System.out.println("false");
        }
    }
}