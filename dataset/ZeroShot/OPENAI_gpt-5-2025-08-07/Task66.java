import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.*;

public class Task66 {

    private static final Set<String> ALLOWED_CMDS = new HashSet<>(Arrays.asList(
            "echo", "date", "uname", "whoami", "pwd", "ls"
    ));
    private static final Set<String> UNAME_OPTS = new HashSet<>(Arrays.asList(
            "-a", "-s", "-r", "-m", "-n"
    ));
    private static final Set<String> LS_OPTS = new HashSet<>(Arrays.asList(
            "-l", "-a", "-la", "-al", "-lah", "-hal", "-hla", "-alh"
    ));
    private static final int MAX_INPUT_LEN = 200;
    private static final int MAX_OUTPUT_BYTES = 8192;
    private static final long TIMEOUT_SECONDS = 5;

    public static String runSafeCommand(String input) {
        if (input == null) return "INVALID: empty";
        String trimmed = input.trim();
        if (trimmed.isEmpty()) return "INVALID: empty";
        if (trimmed.length() > MAX_INPUT_LEN) return "INVALID: too long";
        if (!allCharsAllowed(trimmed)) return "INVALID: contains disallowed characters";

        String[] parts = trimmed.split("\\s+");
        if (parts.length == 0) return "INVALID: empty";
        String cmd = parts[0];
        if (!ALLOWED_CMDS.contains(cmd)) return "INVALID: command not allowed";

        List<String> args = new ArrayList<>();
        args.add(cmd);
        switch (cmd) {
            case "echo": {
                for (int i = 1; i < parts.length; i++) {
                    String t = parts[i];
                    if (t.startsWith("-")) return "INVALID: echo options not allowed";
                    if (!safeToken(t)) return "INVALID: unsafe token";
                    args.add(t);
                }
                break;
            }
            case "date": {
                if (parts.length > 1) return "INVALID: date takes no arguments";
                break;
            }
            case "uname": {
                if (parts.length > 2) return "INVALID: too many args for uname";
                if (parts.length == 2) {
                    String opt = parts[1];
                    if (!UNAME_OPTS.contains(opt)) return "INVALID: uname option not allowed";
                    args.add(opt);
                }
                break;
            }
            case "whoami":
            case "pwd": {
                if (parts.length > 1) return "INVALID: command takes no arguments";
                break;
            }
            case "ls": {
                boolean pathSeen = false;
                for (int i = 1; i < parts.length; i++) {
                    String t = parts[i];
                    if (t.startsWith("-")) {
                        if (!LS_OPTS.contains(t)) return "INVALID: ls option not allowed";
                        args.add(t);
                    } else {
                        if (pathSeen) return "INVALID: only one path allowed";
                        if (!safePath(t)) return "INVALID: unsafe path";
                        args.add(t);
                        pathSeen = true;
                    }
                }
                break;
            }
            default:
                return "INVALID: command not allowed";
        }

        List<String> cmdLine = new ArrayList<>(args.size());
        // Use command name; avoid shell. Optionally map to absolute paths if desired.
        cmdLine.addAll(args);

        ProcessBuilder pb = new ProcessBuilder(cmdLine);
        pb.redirectErrorStream(true);

        Process process;
        try {
            process = pb.start();
        } catch (IOException e) {
            return "ERROR: cannot start process";
        }

        ExecutorService es = Executors.newSingleThreadExecutor(r -> {
            Thread t = new Thread(r);
            t.setDaemon(true);
            return t;
        });

        Callable<String> readTask = () -> {
            StringBuilder sb = new StringBuilder();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
                int total = 0;
                char[] buf = new char[1024];
                int n;
                while ((n = br.read(buf)) != -1) {
                    total += n;
                    if (total > MAX_OUTPUT_BYTES) {
                        sb.append(new String(buf, 0, n));
                        sb.append("\n[truncated]");
                        break;
                    }
                    sb.append(buf, 0, n);
                }
            } catch (IOException ignored) {}
            return sb.toString();
        };

        Future<String> fut = es.submit(readTask);
        boolean finished;
        try {
            finished = process.waitFor(TIMEOUT_SECONDS, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            process.destroyForcibly();
            es.shutdownNow();
            Thread.currentThread().interrupt();
            return "ERROR: interrupted";
        }
        if (!finished) {
            process.destroyForcibly();
            es.shutdownNow();
            return "ERROR: timeout";
        }
        String out;
        try {
            out = fut.get(1, TimeUnit.SECONDS);
        } catch (Exception e) {
            es.shutdownNow();
            return "ERROR: read failure";
        }
        es.shutdown();
        return out == null ? "" : out;
    }

    private static boolean allCharsAllowed(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (Character.isLetterOrDigit(c)) continue;
            switch (c) {
                case ' ': case '.': case '_': case '/': case '-':
                case ':': case '=': case '+': case ',': 
                    continue;
                default:
                    return false;
            }
        }
        return true;
    }

    private static boolean safeToken(String t) {
        if (t.isEmpty() || t.length() > 128) return false;
        for (int i = 0; i < t.length(); i++) {
            char c = t.charAt(i);
            if (Character.isLetterOrDigit(c)) continue;
            switch (c) {
                case '.': case '_': case '/': case '-':
                case ':': case '=': case '+': case ',':
                    continue;
                default:
                    return false;
            }
        }
        return true;
    }

    private static boolean safePath(String p) {
        if (!safeToken(p)) return false;
        if (p.startsWith("/")) return false;
        if (p.contains("..")) return false;
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "echo hello",
                "date",
                "uname -a",
                "ls -l",
                "pwd"
        };
        for (String t : tests) {
            String res = runSafeCommand(t);
            System.out.println("Command: " + t);
            System.out.println("Output:\n" + res);
            System.out.println("----");
        }
    }
}