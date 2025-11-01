import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse101 {
    public static boolean writeImportantConfig(String content) {
        String fixedContent = "important_config";
        Path p = Paths.get("important_config");
        try {
            if (Files.exists(p, LinkOption.NOFOLLOW_LINKS)) {
                if (Files.isSymbolicLink(p)) {
                    return false;
                }
            } else {
                try {
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    Files.createFile(p, PosixFilePermissions.asFileAttribute(perms));
                } catch (UnsupportedOperationException e) {
                    Files.createFile(p);
                    File f = p.toFile();
                    f.setReadable(false, false);
                    f.setWritable(false, false);
                    f.setExecutable(false, false);
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                }
            }

            try (SeekableByteChannel ch = Files.newByteChannel(
                    p,
                    StandardOpenOption.WRITE,
                    StandardOpenOption.TRUNCATE_EXISTING)) {
                ByteBuffer buf = StandardCharsets.UTF_8.encode(fixedContent);
                while (buf.hasRemaining()) {
                    ch.write(buf);
                }
            }

            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(p, perms);
            } catch (UnsupportedOperationException e) {
                File f = p.toFile();
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);
            }

            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println(writeImportantConfig("important_config"));
        System.out.println(writeImportantConfig("important_config"));
        System.out.println(writeImportantConfig("important_config"));
        System.out.println(writeImportantConfig("important_config"));
        System.out.println(writeImportantConfig("important_config"));
    }
}