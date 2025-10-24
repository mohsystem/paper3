import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.Optional;

public class Task75 {

    // Return codes:
    // 0 = success, 1 = invalid input, 2 = refused to terminate self, 3 = not found or access denied, 4 = system error
    public static int terminateProcess(long pid, boolean force) {
        try {
            if (pid <= 0L || pid > 4_000_000_000L) {
                return 1;
            }
            long selfPid = ProcessHandle.current().pid();
            if (pid == selfPid) {
                return 2;
            }
            Optional<ProcessHandle> oh = ProcessHandle.of(pid);
            if (!oh.isPresent()) {
                return 3;
            }
            ProcessHandle ph = oh.get();
            boolean ok = force ? ph.destroyForcibly() : ph.destroy();
            return ok ? 0 : 3;
        } catch (SecurityException se) {
            return 3;
        } catch (Throwable t) {
            return 4;
        }
    }

    private static void printResult(long pid, boolean force, int code) {
        System.out.println(String.format(Locale.ROOT,
                "Attempt terminate pid=%d force=%s -> result=%d",
                pid, Boolean.toString(force), code));
    }

    private static boolean isForceFlag(String s) {
        if (s == null) return false;
        String v = s.trim().toLowerCase(Locale.ROOT);
        return "-f".equals(v) || "--force".equals(v);
    }

    private static Long parsePid(String s) {
        try {
            if (s == null) return null;
            String t = s.trim();
            if (t.isEmpty()) return null;
            long v = Long.parseLong(t);
            return v;
        } catch (NumberFormatException nfe) {
            return null;
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length >= 1) {
            Long pid = parsePid(args[0]);
            boolean force = args.length >= 2 && isForceFlag(args[1]);
            if (pid == null) {
                System.out.println("Invalid PID input. Usage: java Task75 <pid> [--force|-f]");
                System.exit(1);
                return;
            }
            int code = terminateProcess(pid, force);
            printResult(pid, force, code);
            System.exit(code == 0 ? 0 : 1);
        } else {
            // 5 Test cases
            long self = ProcessHandle.current().pid();
            long[] testPids = new long[] { -5L, 0L, self, 999_999_999L, 1L };
            boolean[] forces = new boolean[] { false, true, false, true, false };
            for (int i = 0; i < 5; i++) {
                int code = terminateProcess(testPids[i], forces[i]);
                printResult(testPids[i], forces[i], code);
            }
        }
    }
}