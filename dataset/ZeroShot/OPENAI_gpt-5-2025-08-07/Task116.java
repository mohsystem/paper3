import java.io.BufferedWriter;
import java.io.File;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Map;
import java.util.regex.Pattern;

public class Task116 {

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-z_][a-z0-9_-]{0,31}$");

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (!USERNAME_PATTERN.matcher(username).matches()) return false;
        return true;
    }

    private static boolean isStrongPassword(String pwd) {
        if (pwd == null) return false;
        if (pwd.length() < 8 || pwd.length() > 128) return false;
        return true;
    }

    private static String findChpasswdPath() {
        String[] candidates = new String[] {
            "/usr/sbin/chpasswd",
            "/sbin/chpasswd",
            "/usr/bin/chpasswd",
            "/bin/chpasswd"
        };
        for (String p : candidates) {
            File f = new File(p);
            if (f.exists() && f.canExecute()) return p;
        }
        return null;
    }

    private static boolean isRoot() {
        try {
            ProcessBuilder pb = new ProcessBuilder("/usr/bin/id", "-u");
            Map<String, String> env = pb.environment();
            env.clear();
            env.put("PATH", "/usr/sbin:/usr/bin:/sbin:/bin");
            env.put("LC_ALL", "C");
            pb.redirectErrorStream(true);
            Process proc = pb.start();
            String output = new String(proc.getInputStream().readAllBytes(), StandardCharsets.UTF_8).trim();
            int rc = proc.waitFor();
            if (rc == 0 && "0".equals(output)) {
                return true;
            }
        } catch (Exception ignored) {
        }
        return false;
    }

    private static boolean changePasswordPrivileged(String username, char[] newPassword) {
        String chpasswd = findChpasswdPath();
        if (chpasswd == null) return false;
        Process proc = null;
        try {
            ProcessBuilder pb = new ProcessBuilder(chpasswd);
            Map<String, String> env = pb.environment();
            env.clear();
            env.put("PATH", "/usr/sbin:/usr/bin:/sbin:/bin");
            env.put("LC_ALL", "C");
            pb.redirectErrorStream(true);
            proc = pb.start();
            try (BufferedWriter w = new BufferedWriter(new OutputStreamWriter(proc.getOutputStream(), StandardCharsets.UTF_8))) {
                w.write(username);
                w.write(':');
                w.write(newPassword);
                w.write('\n');
                w.flush();
            }
            int rc = proc.waitFor();
            return rc == 0;
        } catch (Exception e) {
            return false;
        } finally {
            if (proc != null) {
                proc.destroy();
            }
            if (newPassword != null) {
                Arrays.fill(newPassword, '\0');
            }
        }
    }

    // Note: Java cannot drop process privileges (setuid/setgid) without native calls (JNI/JNA).
    // As a mitigation, we can spawn a subprocess under a less-privileged user for subsequent work.
    private static boolean simulateDropPrivilegesToUser(String dropUser) {
        if (dropUser == null || dropUser.isEmpty()) dropUser = "nobody";
        try {
            if (!isRoot()) {
                // Not root; nothing to drop; consider this step non-failing in this context.
                return true;
            }
            ProcessBuilder pb = new ProcessBuilder("/sbin/runuser", "-u", dropUser, "--", "/usr/bin/id", "-u");
            Map<String, String> env = pb.environment();
            env.clear();
            env.put("PATH", "/usr/sbin:/usr/bin:/sbin:/bin");
            env.put("LC_ALL", "C");
            pb.redirectErrorStream(true);
            Process proc = pb.start();
            int rc = proc.waitFor();
            return rc == 0;
        } catch (Exception e) {
            return false;
        }
    }

    public static boolean changePasswordAndDrop(String username, String newPassword, String dropToUser, boolean dryRun) {
        if (!isValidUsername(username)) return false;
        if (!isStrongPassword(newPassword)) return false;

        char[] pwd = newPassword.toCharArray();
        boolean ok = true;

        try {
            if (!dryRun) {
                if (!isRoot()) return false;
                ok = changePasswordPrivileged(username, pwd);
                if (!ok) return false;
            }
        } finally {
            Arrays.fill(pwd, '\0');
        }

        // Simulate privilege drop by running a subprocess under a low-privilege user
        boolean dropOk = simulateDropPrivilegesToUser(dropToUser != null ? dropToUser : "nobody");
        return ok && dropOk;
    }

    public static void main(String[] args) {
        // 5 test cases (all dry-run to avoid real system changes)
        System.out.println(changePasswordAndDrop("testuser", "CorrectHorse1!", "nobody", true));
        System.out.println(changePasswordAndDrop("root", "XyZ12345!", "nobody", true));
        System.out.println(changePasswordAndDrop("bad:user", "StrongPassw0rd!", "nobody", true));
        System.out.println(changePasswordAndDrop("good_user", "short", "nobody", true));
        System.out.println(changePasswordAndDrop("alice", "Secur3P@ssword!", "nobody", true));
    }
}