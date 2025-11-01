import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse101 {
    public static boolean writeImportantConfig(String path, String content) {
        Path p = Paths.get(path);
        try (OutputStream os = Files.newOutputStream(p,
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.WRITE)) {
            os.write(content.getBytes(StandardCharsets.UTF_8));
        } catch (IOException e) {
            return false;
        }

        boolean permsSet = true;
        try {
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
                Files.setPosixFilePermissions(p, perms);
            } else {
                File f = p.toFile();
                boolean r = f.setReadable(true, false);
                boolean w = f.setWritable(true, true);
                boolean x = f.setExecutable(false, false);
                permsSet = r && w && x;
            }
        } catch (Exception e) {
            permsSet = false;
        }
        return permsSet;
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            boolean ok = writeImportantConfig("important_config", "important_config");
            System.out.println("Test " + i + ": " + ok);
        }
    }
}