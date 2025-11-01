import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse105 {
    public static boolean createRestrictiveFile(String path, String content) {
        try {
            Path p = Paths.get(path);
            Files.write(p, content.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            try {
                PosixFileAttributeView view = Files.getFileAttributeView(p, PosixFileAttributeView.class);
                if (view != null) {
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    Files.setPosixFilePermissions(p, perms);
                } else {
                    File f = p.toFile();
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                    f.setExecutable(false, false);
                }
            } catch (UnsupportedOperationException e) {
                File f = p.toFile();
                f.setReadable(true, true);
                f.setWritable(true, true);
                f.setExecutable(false, false);
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        createRestrictiveFile("secret.txt", content);
        createRestrictiveFile("secret1.txt", content);
        createRestrictiveFile("secret2.txt", content);
        createRestrictiveFile("secret3.txt", content);
        createRestrictiveFile("secret4.txt", content);
    }
}