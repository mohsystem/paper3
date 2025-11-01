import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse107 {
    public static String secureFile(String filename, String content, int octalPerm) throws IOException {
        Path path = Paths.get(filename);
        Files.write(path, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

        boolean posixSupported = Files.getFileAttributeView(path, PosixFileAttributeView.class) != null;

        if (posixSupported) {
            Set<PosixFilePermission> perms = permsFromMode(octalPerm);
            Files.setPosixFilePermissions(path, perms);
            Set<PosixFilePermission> got = Files.getPosixFilePermissions(path);
            int mode = modeFromPerms(got);
            return String.format("%03o %s", mode, permStringFromMode(mode));
        } else {
            File f = path.toFile();
            boolean ownerRead = (octalPerm & 0400) != 0;
            boolean ownerWrite = (octalPerm & 0200) != 0;
            boolean ownerExec = (octalPerm & 0100) != 0;

            // Best-effort on non-POSIX: set owner-only flags
            f.setReadable(ownerRead, true);
            f.setWritable(ownerWrite, true);
            f.setExecutable(ownerExec, true);

            // Report intended mode since exact retrieval may not be supported
            return String.format("%03o %s", (octalPerm & 0777), permStringFromMode(octalPerm));
        }
    }

    private static Set<PosixFilePermission> permsFromMode(int mode) {
        Set<PosixFilePermission> perms = EnumSet.noneOf(PosixFilePermission.class);
        if ((mode & 0400) != 0) perms.add(PosixFilePermission.OWNER_READ);
        if ((mode & 0200) != 0) perms.add(PosixFilePermission.OWNER_WRITE);
        if ((mode & 0100) != 0) perms.add(PosixFilePermission.OWNER_EXECUTE);
        if ((mode & 0040) != 0) perms.add(PosixFilePermission.GROUP_READ);
        if ((mode & 0020) != 0) perms.add(PosixFilePermission.GROUP_WRITE);
        if ((mode & 0010) != 0) perms.add(PosixFilePermission.GROUP_EXECUTE);
        if ((mode & 0004) != 0) perms.add(PosixFilePermission.OTHERS_READ);
        if ((mode & 0002) != 0) perms.add(PosixFilePermission.OTHERS_WRITE);
        if ((mode & 0001) != 0) perms.add(PosixFilePermission.OTHERS_EXECUTE);
        return perms;
    }

    private static int modeFromPerms(Set<PosixFilePermission> perms) {
        int mode = 0;
        if (perms.contains(PosixFilePermission.OWNER_READ)) mode |= 0400;
        if (perms.contains(PosixFilePermission.OWNER_WRITE)) mode |= 0200;
        if (perms.contains(PosixFilePermission.OWNER_EXECUTE)) mode |= 0100;
        if (perms.contains(PosixFilePermission.GROUP_READ)) mode |= 0040;
        if (perms.contains(PosixFilePermission.GROUP_WRITE)) mode |= 0020;
        if (perms.contains(PosixFilePermission.GROUP_EXECUTE)) mode |= 0010;
        if (perms.contains(PosixFilePermission.OTHERS_READ)) mode |= 0004;
        if (perms.contains(PosixFilePermission.OTHERS_WRITE)) mode |= 0002;
        if (perms.contains(PosixFilePermission.OTHERS_EXECUTE)) mode |= 0001;
        return mode & 0777;
    }

    private static String permStringFromMode(int mode) {
        StringBuilder sb = new StringBuilder(9);
        int[] bits = {0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001};
        char[] chars = {'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x'};
        for (int i = 0; i < 9; i++) {
            sb.append((mode & bits[i]) != 0 ? chars[i] : '-');
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        String filename = "secret.txt";
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        int mode = 0600;

        System.out.println(secureFile(filename, content, mode));
        System.out.println(secureFile(filename, content, mode));
        System.out.println(secureFile(filename, content, mode));
        System.out.println(secureFile(filename, content, mode));
        System.out.println(secureFile(filename, content, mode));
    }
}