import java.io.File;
import java.io.IOException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse100 {
    public static boolean ensureConfigFile(String filename) {
        Path p = Paths.get(filename);
        try {
            if (Files.notExists(p)) {
                try {
                    if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                        Set<PosixFilePermission> perms = EnumSet.of(
                                PosixFilePermission.OWNER_READ,
                                PosixFilePermission.OWNER_WRITE
                        );
                        Files.createFile(p, PosixFilePermissions.asFileAttribute(perms));
                    } else {
                        Files.createFile(p);
                    }
                } catch (FileAlreadyExistsException ignore) {
                    // proceed to set permissions
                }
            }
            try {
                if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE
                    );
                    Files.setPosixFilePermissions(p, perms);
                } else {
                    File f = p.toFile();
                    f.setReadable(false, false);
                    f.setWritable(false, false);
                    f.setExecutable(false, false);
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                }
            } catch (UnsupportedOperationException ignore) {
                // best effort on non-POSIX systems
            }
            return Files.exists(p);
        } catch (IOException e) {
            return false;
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            boolean ok = ensureConfigFile("important_config");
            System.out.println("Test " + i + ": " + ok);
        }
    }
}