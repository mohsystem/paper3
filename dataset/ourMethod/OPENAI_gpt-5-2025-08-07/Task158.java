import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Task158 {

    public static String runSafeEcho(String input) {
        if (!isValidInput(input)) {
            return "ERROR: Invalid input. Allowed: 1-64 chars [A-Za-z0-9 _:-.]";
        }

        List<String> cmd = new ArrayList<>();
        if (isWindows()) {
            // Use PowerShell with argument vector to avoid shell injection
            cmd.add("powershell");
            cmd.add("-NoProfile");
            cmd.add("-NonInteractive");
            cmd.add("-Command");
            cmd.add("Write-Output");
            cmd.add(input);
        } else {
            String printfPath = findExecutable(new String[]{"/usr/bin/printf", "/bin/printf"});
            if (printfPath == null) {
                return "ERROR: printf not found on this system.";
            }
            cmd.add(printfPath);
            cmd.add("%s\n");
            cmd.add(input);
        }

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);

        try {
            Process p = pb.start();
            boolean finished = p.waitFor(2, TimeUnit.SECONDS);
            if (!finished) {
                p.destroyForcibly();
                return "ERROR: Command timed out.";
            }
            try (InputStream is = new BufferedInputStream(p.getInputStream());
                 ByteArrayOutputStream bos = new ByteArrayOutputStream()) {
                byte[] buf = new byte[4096];
                int r;
                while ((r = is.read(buf)) != -1) {
                    bos.write(buf, 0, r);
                }
                return bos.toString(StandardCharsets.UTF_8);
            }
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static boolean isWindows() {
        String os = System.getProperty("os.name");
        return os != null && os.toLowerCase().contains("win");
    }

    private static String findExecutable(String[] candidates) {
        for (String c : candidates) {
            try {
                if (Files.isExecutable(Path.of(c))) {
                    return c;
                }
            } catch (Exception ignored) {}
        }
        return null;
    }

    private static boolean isValidInput(String s) {
        if (s == null) return false;
        if (s.length() < 1 || s.length() > 64) return false;
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            boolean ok =
                (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= '0' && ch <= '9') ||
                ch == ' ' || ch == '_' || ch == '-' || ch == ':' || ch == '.';
            if (!ok) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "HelloWorld",
            "Safe_123",
            "alpha-beta",
            "COLON:TEST",
            "Numbers 0123456789"
        };
        for (String t : tests) {
            String out = runSafeEcho(t);
            System.out.println("Input: " + t);
            System.out.println("Output: " + out);
            System.out.println("-----");
        }
    }
}