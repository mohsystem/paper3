import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.channels.FileChannel;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse104 {
    public static int writeConfigSecure(String filePath, String content) throws IOException {
        if (filePath == null) {
            throw new IllegalArgumentException("filePath cannot be null");
        }
        if (content == null) {
            content = "";
        }

        Path p = Paths.get(filePath);
        if (p.getParent() != null) {
            Files.createDirectories(p.getParent());
        }

        byte[] data = content.getBytes(StandardCharsets.UTF_8);

        EnumSet<StandardOpenOption> options = EnumSet.of(
                StandardOpenOption.READ,
                StandardOpenOption.WRITE,
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING
        );

        boolean isPosix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
        try (FileChannel fc = isPosix
                ? FileChannel.open(p, options, posix0600())
                : FileChannel.open(p, options)) {

            if (!isPosix) {
                // Best-effort on non-POSIX systems to restrict permissions
                File f = p.toFile();
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);
            }

            ByteBuffer buf = ByteBuffer.wrap(data);
            while (buf.hasRemaining()) {
                fc.write(buf);
            }
            fc.force(true);
        }
        return data.length;
    }

    private static FileAttribute<Set<PosixFilePermission>> posix0600() {
        Set<PosixFilePermission> perms = EnumSet.of(
                PosixFilePermission.OWNER_READ,
                PosixFilePermission.OWNER_WRITE
        );
        return PosixFilePermissions.asFileAttribute(perms);
    }

    public static void main(String[] args) {
        String tmp = System.getProperty("java.io.tmpdir");
        String[] files = new String[] {
                Paths.get(tmp, "lse104_java_test1.cfg").toString(),
                Paths.get(tmp, "lse104_java_test2.cfg").toString(),
                Paths.get(tmp, "lse104_java_test3.cfg").toString(),
                Paths.get(tmp, "lse104_java_test4.cfg").toString(),
                Paths.get(tmp, "lse104_java_test5.cfg").toString()
        };

        for (int i = 0; i < files.length; i++) {
            try {
                int written = writeConfigSecure(files[i], "important_config");
                System.out.println("Test " + (i+1) + " wrote " + written + " bytes to " + files[i]);
            } catch (IOException e) {
                System.err.println("Test " + (i+1) + " failed for " + files[i] + ": " + e.getMessage());
            }
        }
    }
}