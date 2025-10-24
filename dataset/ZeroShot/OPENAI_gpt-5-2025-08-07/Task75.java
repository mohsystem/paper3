import java.util.Optional;
import java.util.concurrent.TimeUnit;

public class Task75 {

    public static boolean terminateProcess(long pid) {
        if (pid <= 1) {
            return false;
        }
        long self = ProcessHandle.current().pid();
        if (pid == self) {
            return false;
        }

        Optional<ProcessHandle> opt = ProcessHandle.of(pid);
        if (!opt.isPresent()) {
            return false;
        }

        ProcessHandle ph = opt.get();

        // Avoid killing our direct parent process for safety (optional safeguard)
        try {
            if (ph.parent().isPresent() && ph.parent().get().pid() == self) {
                return false;
            }
        } catch (SecurityException ignored) {
        }

        if (!ph.isAlive()) {
            return true;
        }

        boolean destroyed = false;
        try {
            destroyed = ph.destroy();
            if (!destroyed) {
                destroyed = ph.destroyForcibly();
            }
            if (destroyed) {
                try {
                    ph.onExit().get(5, TimeUnit.SECONDS);
                } catch (Exception ignored) {
                }
            }
        } catch (SecurityException ignored) {
            destroyed = false;
        }

        try {
            return !ph.isAlive();
        } catch (SecurityException e) {
            // If we cannot query status after attempting destruction, report best effort result
            return destroyed;
        }
    }

    private static long parsePid(String s) {
        try {
            long v = Long.parseLong(s);
            return v;
        } catch (NumberFormatException e) {
            return Long.MIN_VALUE;
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            for (String a : args) {
                long pid = parsePid(a);
                if (pid == Long.MIN_VALUE) {
                    System.out.println("PID \"" + a + "\" is invalid input.");
                    continue;
                }
                boolean result = terminateProcess(pid);
                System.out.println("Terminate PID " + pid + ": " + (result ? "success" : "failed"));
            }
        } else {
            // 5 test cases
            long self = ProcessHandle.current().pid();
            long[] tests = new long[] { -5, 0, 1, self, 99999999L };
            for (long pid : tests) {
                boolean result = terminateProcess(pid);
                System.out.println("Test terminate PID " + pid + ": " + (result ? "success" : "failed"));
            }
        }
    }
}