import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

public class Task116 {
    // For safety, this method simulates a privileged operation (e.g., changing a password)
    // and demonstrates privilege dropping via subprocesses; it does not actually modify system passwords.
    public static String performPrivilegedAndDrop(String targetUser, String newPassword, String dropToUser) {
        StringBuilder result = new StringBuilder();
        boolean isRoot = isRoot();
        result.append("isRoot=").append(isRoot).append("; ");

        try {
            long pid = ProcessHandle.current().pid();
            Path p = Paths.get("/tmp/task116_java_priv_" + pid);
            Files.write(p, ("Simulated privileged content for " + targetUser + "\n").getBytes(StandardCharsets.UTF_8));
            if (isPosix()) {
                Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                try {
                    Files.setPosixFilePermissions(p, perms);
                } catch (UnsupportedOperationException ignored) {}
            }
            result.append("createdFile=").append(p.toString()).append("; ");

            // "Dropping privileges" within JVM isn't supported directly; simulate by running a command as another user.
            if (isRoot) {
                int dropUid = uidOf(dropToUser);
                if (dropUid < 0) {
                    result.append("dropToUserNotFound=").append(dropToUser).append("; ");
                } else {
                    String[] cmd = new String[] { "sh", "-lc", "runuser -u " + escape(dropToUser) + " -- sh -lc 'id -u; cat " + escape(p.toString()) + " >/dev/null 2>&1; echo $?'" };
                    CmdOut co = run(cmd);
                    result.append("droppedSubprocessUid=").append(co.stdoutLines.length > 0 ? co.stdoutLines[0].trim() : "NA").append("; ");
                    if (co.stdoutLines.length > 1) {
                        result.append("readFileAsDroppedUserExit=").append(co.stdoutLines[1].trim()).append("; ");
                    }
                    result.append("dropExitCode=").append(co.exitCode).append("; ");
                }
            } else {
                result.append("notRootNoRealDrop; ");
            }
        } catch (Exception e) {
            result.append("error=").append(e.getClass().getSimpleName()).append(":").append(e.getMessage()).append("; ");
        }
        return result.toString();
    }

    private static boolean isPosix() {
        String os = System.getProperty("os.name", "").toLowerCase();
        return os.contains("nix") || os.contains("nux") || os.contains("mac");
    }

    private static boolean isRoot() {
        try {
            CmdOut co = run(new String[] { "sh", "-lc", "id -u" });
            if (co.exitCode == 0 && co.stdoutLines.length > 0) {
                return "0".equals(co.stdoutLines[0].trim());
            }
        } catch (Exception ignored) {}
        return false;
    }

    private static int uidOf(String user) {
        try {
            CmdOut co = run(new String[] { "sh", "-lc", "id -u " + escape(user) });
            if (co.exitCode == 0 && co.stdoutLines.length > 0) {
                return Integer.parseInt(co.stdoutLines[0].trim());
            }
        } catch (Exception ignored) {}
        return -1;
    }

    private static String escape(String s) {
        return s.replace("'", "'\"'\"'");
    }

    private static class CmdOut {
        String[] stdoutLines;
        String[] stderrLines;
        int exitCode;
    }

    private static CmdOut run(String[] cmd) throws Exception {
        Process p = new ProcessBuilder(cmd).redirectErrorStream(false).start();
        StringBuilder out = new StringBuilder();
        StringBuilder err = new StringBuilder();
        try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
            String line;
            while ((line = r.readLine()) != null) out.append(line).append("\n");
        }
        try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getErrorStream(), StandardCharsets.UTF_8))) {
            String line;
            while ((line = r.readLine()) != null) err.append(line).append("\n");
        }
        int ec = p.waitFor();
        CmdOut co = new CmdOut();
        co.stdoutLines = out.toString().isEmpty() ? new String[0] : out.toString().split("\n");
        co.stderrLines = err.toString().isEmpty() ? new String[0] : err.toString().split("\n");
        co.exitCode = ec;
        return co;
    }

    public static void main(String[] args) {
        // 5 test cases (simulation only)
        System.out.println(performPrivilegedAndDrop("alice", "Secret123!", "nobody"));
        System.out.println(performPrivilegedAndDrop("bob", "P@ssw0rd!", "daemon"));
        System.out.println(performPrivilegedAndDrop("carol", "NewPass#1", "nonexistentuser"));
        System.out.println(performPrivilegedAndDrop("dave", "Another#2", "nobody"));
        System.out.println(performPrivilegedAndDrop("root", "RootPass#3", "daemon"));
    }
}