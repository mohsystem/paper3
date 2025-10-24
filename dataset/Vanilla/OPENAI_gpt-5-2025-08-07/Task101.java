import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Task101 {
    public static boolean createExecutableScript(String dirPath, String content) {
        try {
            if (content == null || content.isEmpty()) {
                content = "#!/bin/bash\n" +
                          "echo \"Hello from script.sh\"\n";
            }
            Path dir = (dirPath == null || dirPath.isEmpty() || ".".equals(dirPath))
                    ? Paths.get(".")
                    : Paths.get(dirPath);
            Path scriptPath = dir.resolve("script.sh");

            if (scriptPath.getParent() != null) {
                Files.createDirectories(scriptPath.getParent());
            }

            try (BufferedWriter writer = Files.newBufferedWriter(scriptPath)) {
                writer.write(content);
            }

            boolean execSet = false;
            try {
                // Try POSIX permissions if supported
                FileStore store = Files.getFileStore(scriptPath);
                if (store.supportsFileAttributeView("posix")) {
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    perms.add(PosixFilePermission.OWNER_EXECUTE);
                    perms.add(PosixFilePermission.GROUP_READ);
                    perms.add(PosixFilePermission.GROUP_EXECUTE);
                    perms.add(PosixFilePermission.OTHERS_READ);
                    perms.add(PosixFilePermission.OTHERS_EXECUTE);
                    Files.setPosixFilePermissions(scriptPath, perms);
                    execSet = true;
                }
            } catch (UnsupportedOperationException ignored) {
            }

            if (!execSet) {
                File f = scriptPath.toFile();
                // setExecutable(executable, ownerOnly). ownerOnly=false -> all users
                execSet = f.setExecutable(true, false);
            }

            return execSet;
        } catch (IOException e) {
            return false;
        }
    }

    public static void main(String[] args) {
        String[] dirs = {
                ".",
                "java_out1",
                "java_out2/nested",
                "java out3/spaced",
                "java_out4/a/b"
        };
        for (int i = 0; i < dirs.length; i++) {
            String content = "#!/bin/bash\n" +
                    "echo \"Java test " + (i + 1) + " in " + dirs[i].replace("\"", "\\\"") + "\"\n";
            boolean ok = createExecutableScript(dirs[i], content);
            System.out.println("Test " + (i + 1) + " (" + dirs[i] + "): " + (ok ? "success" : "failed"));
        }
    }
}