// Chain-of-Through steps applied:
// 1) Problem understanding: Implement a safe executor that validates a bash-like command string, only allows a strict subset of commands/arguments, runs without invoking a shell, returns combined output.
// 2) Security requirements: No shell interpretation, strict allowlist, reject metacharacters, limit length/args/output/time, resolve to absolute binaries, avoid environment injection.
// 3) Secure coding generation: Implement careful validation and execution with ProcessBuilder; merge stderr/stdout; use timeout and output caps.
// 4) Code review notes: No shell, no unbounded reads, careful char checks, path resolution and argument restrictions enforced.
// 5) Secure code output: Finalized after applying the above controls.

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.TimeUnit;

public class Task66 {

    private static final int MAX_CMD_LEN = 256;
    private static final int MAX_ARGS = 8;
    private static final int MAX_OUTPUT_BYTES = 1_000_000;
    private static final long TIMEOUT_SEC = 3;

    private static final Set<String> ALLOWED_CMDS = new HashSet<>(Arrays.asList(
            "echo", "ls", "pwd", "whoami", "date", "uname", "cat"
    ));

    // Characters not allowed anywhere
    private static final String FORBIDDEN_CHARS = ";|&><`$(){}[]\\'\"*!?~\n\r\t";

    // Allowed characters for tokens (conservative)
    private static boolean isSafeToken(String t) {
        if (t.isEmpty() || t.length() > 128) return false;
        for (int i = 0; i < t.length(); i++) {
            char c = t.charAt(i);
            if (c < 32 || c > 126) return false; // disallow control/non-ascii
            if (FORBIDDEN_CHARS.indexOf(c) >= 0) return false;
            // allow only: alnum and . _ / - = : @ , + %
            if (!(Character.isLetterOrDigit(c) ||
                 c == '.' || c == '_' || c == '/' || c == '-' ||
                 c == '=' || c == ':' || c == '@' || c == ',' ||
                 c == '+' || c == '%')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isSafePathArg(String t) {
        if (!isSafeToken(t)) return false;
        if (t.startsWith("/")) return false; // no absolute paths
        if (t.startsWith("-")) return false; // avoid ambiguous flags as path
        if (t.contains("..")) return false;  // no parent traversal
        return true;
    }

    private static boolean isLsFlag(String t) {
        if (t.length() < 2 || t.charAt(0) != '-') return false;
        for (int i = 1; i < t.length(); i++) {
            char c = t.charAt(i);
            if ("lah1".indexOf(c) < 0) return false;
        }
        return true;
    }

    private static boolean validateTokens(List<String> tokens) {
        if (tokens.isEmpty()) return false;
        if (tokens.size() > MAX_ARGS) return false;
        String cmd = tokens.get(0);
        if (!ALLOWED_CMDS.contains(cmd)) return false;

        // Validate per-command args
        List<String> args = tokens.subList(1, tokens.size());
        switch (cmd) {
            case "pwd":
            case "whoami":
            case "date":
                return args.isEmpty();
            case "uname":
                return args.isEmpty() || (args.size() == 1 && "-a".equals(args.get(0)));
            case "echo":
                if (args.size() > 5) return false;
                for (String a : args) if (!isSafeToken(a)) return false;
                return true;
            case "ls":
                if (args.size() > 2) return false;
                boolean sawPath = false;
                for (String a : args) {
                    if (a.startsWith("-")) {
                        if (!isLsFlag(a)) return false;
                    } else {
                        if (sawPath) return false; // at most one path
                        if (!isSafePathArg(a)) return false;
                        sawPath = true;
                    }
                }
                return true;
            case "cat":
                // Only allow cat on safe relative paths
                if (args.isEmpty() || args.size() > 2) return false;
                for (String a : args) {
                    if (!isSafePathArg(a)) return false;
                }
                return true;
            default:
                return false;
        }
    }

    private static List<String> tokenize(String input) {
        String[] parts = input.trim().split("\\s+");
        List<String> tokens = new ArrayList<>();
        for (String p : parts) if (!p.isEmpty()) tokens.add(p);
        return tokens;
    }

    public static String runSafeCommand(String input) {
        if (input == null) return "Error: Empty input";
        if (input.length() > MAX_CMD_LEN) return "Error: Command too long";
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c < 32 || c > 126) return "Error: Invalid characters present";
            if (FORBIDDEN_CHARS.indexOf(c) >= 0) return "Error: Forbidden characters present";
        }

        List<String> tokens = tokenize(input);
        if (!validateTokens(tokens)) return "Error: Command not allowed or invalid arguments";

        // Resolve absolute path for command (avoid PATH surprises)
        String cmd = tokens.get(0);
        String abs = resolveAbsolute(cmd);
        if (abs == null) return "Error: Command binary not found";
        tokens.set(0, abs);

        ProcessBuilder pb = new ProcessBuilder(tokens);
        pb.redirectErrorStream(true);
        Map<String, String> env = pb.environment();
        // Clear potentially dangerous environment variables
        env.remove("LD_PRELOAD");
        env.remove("LD_LIBRARY_PATH");

        StringBuilder out = new StringBuilder();
        try {
            Process p = pb.start();
            BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8));
            long deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(TIMEOUT_SEC);
            char[] buf = new char[4096];
            while (true) {
                while (r.ready()) {
                    int n = r.read(buf);
                    if (n <= 0) break;
                    if (out.length() + n > MAX_OUTPUT_BYTES) {
                        out.append("\n[Truncated output]");
                        p.destroyForcibly();
                        return out.toString();
                    }
                    out.append(buf, 0, n);
                }
                if (p.waitFor(50, TimeUnit.MILLISECONDS)) {
                    // drain remaining
                    while (r.ready()) {
                        int n = r.read(buf);
                        if (n <= 0) break;
                        if (out.length() + n > MAX_OUTPUT_BYTES) {
                            out.append("\n[Truncated output]");
                            break;
                        }
                        out.append(buf, 0, n);
                    }
                    break;
                }
                if (System.nanoTime() > deadline) {
                    p.destroyForcibly();
                    return "[Error] Execution timed out";
                }
            }
        } catch (Exception e) {
            return "Error: Execution failed";
        }
        return out.toString();
    }

    private static String resolveAbsolute(String cmd) {
        String[] paths = {"/bin/" + cmd, "/usr/bin/" + cmd};
        for (String p : paths) {
            java.io.File f = new java.io.File(p);
            if (f.exists() && f.canExecute()) return p;
        }
        return null;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
                "pwd",
                "whoami",
                "date",
                "echo safe_test",
                "ls -l"
        };
        for (String t : tests) {
            System.out.println(">>> " + t);
            System.out.println(runSafeCommand(t));
        }
    }
}