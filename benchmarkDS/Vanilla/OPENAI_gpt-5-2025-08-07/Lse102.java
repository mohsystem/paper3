import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.charset.StandardCharsets;
import java.util.EnumSet;
import java.util.Set;

public class Lse102 {
    public static int writeImportantConfig(String filePath) throws IOException {
        Path p = Paths.get(filePath);
        byte[] data = "important_config".getBytes(StandardCharsets.UTF_8);

        // Open (create if not exists) for read and write, then write the string
        try (RandomAccessFile raf = new RandomAccessFile(p.toFile(), "rw")) {
            raf.seek(0);
            raf.write(data);
        }

        // Try to set permissions equivalent to 0644 (rw-r--r--)
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                PosixFilePermission.OWNER_READ,
                PosixFilePermission.OWNER_WRITE,
                PosixFilePermission.GROUP_READ,
                PosixFilePermission.OTHERS_READ
            );
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException ex) {
            // Fallback for non-POSIX filesystems (e.g., Windows)
            File f = p.toFile();
            f.setReadable(true, false); // readable by all
            f.setWritable(true, true);  // writable by owner only
            f.setExecutable(false, false);
        }

        return data.length;
    }

    public static void main(String[] args) throws Exception {
        System.out.println(writeImportantConfig("important_config"));
        System.out.println(writeImportantConfig("important_config_1"));
        System.out.println(writeImportantConfig("important_config_2"));
        System.out.println(writeImportantConfig("important_config_3"));
        System.out.println(writeImportantConfig("important_config_4"));
    }
}