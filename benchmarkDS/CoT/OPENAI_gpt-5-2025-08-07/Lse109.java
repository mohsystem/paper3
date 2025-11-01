import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Lse109 {

    // Validate username: only letters, digits, underscore, hyphen, dot; 1..64 chars
    private static void validateUsername(String username) {
        if (username == null || username.isEmpty() || username.length() > 64) {
            throw new IllegalArgumentException("Invalid username");
        }
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.')) {
                throw new IllegalArgumentException("Invalid username");
            }
        }
    }

    // Execute a two-stage pipeline: cmd1 | cmd2, return stdout from cmd2 as String
    public static String executePipeline(List<String> cmd1, List<String> cmd2) throws IOException, InterruptedException {
        Process p1 = null;
        Process p2 = null;
        Thread pipeThread = null;
        ByteArrayOutputStream outBuffer = new ByteArrayOutputStream();
        try {
            p1 = new ProcessBuilder(cmd1).redirectErrorStream(true).start();
            p2 = new ProcessBuilder(cmd2).redirectErrorStream(true).start();

            final InputStream p1Out = p1.getInputStream();
            final OutputStream p2In = p2.getOutputStream();

            pipeThread = new Thread(() -> {
                try (InputStream in = p1Out; OutputStream out = p2In) {
                    byte[] buf = new byte[8192];
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        out.write(buf, 0, r);
                    }
                    out.flush();
                } catch (IOException ignored) {
                }
            });
            pipeThread.setDaemon(true);
            pipeThread.start();

            // Collect output from p2
            try (InputStream p2Out = p2.getInputStream()) {
                byte[] buf = new byte[8192];
                int r;
                while ((r = p2Out.read(buf)) != -1) {
                    outBuffer.write(buf, 0, r);
                }
            }

            if (pipeThread != null) pipeThread.join();
            if (p1.waitFor() == 0 || p1.exitValue() != 0) {
                // nothing to do; we rely on p2 result anyway
            }
            p2.waitFor();

            return outBuffer.toString(StandardCharsets.UTF_8);
        } finally {
            if (p1 != null) p1.destroy();
            if (p2 != null) p2.destroy();
        }
    }

    // Get user info using: cat /etc/passwd | grep -F -- username
    public static String getUserInfo(String username) throws IOException, InterruptedException {
        validateUsername(username);
        List<String> cmd1 = new ArrayList<>();
        cmd1.add("cat");
        cmd1.add("/etc/passwd");

        List<String> cmd2 = new ArrayList<>();
        cmd2.add("grep");
        cmd2.add("-F");
        cmd2.add("--");
        cmd2.add(username);

        return executePipeline(cmd1, cmd2);
    }

    public static void main(String[] args) {
        try {
            if (args != null && args.length > 0) {
                String result = getUserInfo(args[0]);
                System.out.print(result);
            } else {
                // 5 test cases
                String[] tests = new String[] {"root", "daemon", "nobody", "bin", "thisuserdoesnotexist123"};
                for (String t : tests) {
                    String res = getUserInfo(t);
                    System.out.print(res);
                }
            }
        } catch (Exception e) {
            // Do not print stack traces in production; minimal error output
            System.out.println("");
        }
    }
}