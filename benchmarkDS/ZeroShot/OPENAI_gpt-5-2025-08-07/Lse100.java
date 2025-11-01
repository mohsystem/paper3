import java.nio.file.*;
import java.nio.file.attribute.*;
import java.util.*;
import java.io.IOException;
import java.nio.channels.SeekableByteChannel;

public class Lse100 {
    public static boolean createSecureFile(String filename) {
        Path path = Paths.get(filename);

        Set<OpenOption> options = new HashSet<>();
        options.add(StandardOpenOption.CREATE_NEW);
        options.add(StandardOpenOption.WRITE);

        FileAttribute<Set<PosixFilePermission>> attr = null;
        try {
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            attr = PosixFilePermissions.asFileAttribute(perms);
        } catch (Exception e) {
            attr = null;
        }

        SeekableByteChannel ch = null;
        try {
            if (attr != null) {
                ch = Files.newByteChannel(path, options, attr);
            } else {
                ch = Files.newByteChannel(path, options);
            }
            try {
                if (Files.getFileAttributeView(path, PosixFileAttributeView.class) != null) {
                    Files.setPosixFilePermissions(path, PosixFilePermissions.fromString("rw-------"));
                } else {
                    java.io.File f = path.toFile();
                    f.setReadable(false, false);
                    f.setWritable(false, false);
                    f.setExecutable(false, false);
                    f.setReadable(true, true);
                    f.setWritable(true, true);
                }
            } catch (Exception ignored) {}
            return true;
        } catch (FileAlreadyExistsException e) {
            return false;
        } catch (IOException e) {
            return false;
        } finally {
            if (ch != null) {
                try { ch.close(); } catch (IOException ignored) {}
            }
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "important_config",
            "important_config_1",
            "important_config_2",
            "important_config",
            "important_config_3"
        };
        for (String t : tests) {
            boolean ok = createSecureFile(t);
            System.out.println(t + ":" + ok);
        }
    }
}