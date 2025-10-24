import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

public class Task75 {
    // Terminates the process with the given PID.
    // Returns true if the process is not alive at the end (either already dead or successfully terminated).
    public static boolean terminateProcess(long pid) {
        long currentPid = ProcessHandle.current().pid();

        if (pid <= 0) {
            return false; // invalid PID
        }

        // Block terminating self
        if (pid == currentPid) {
            return false;
        }

        // Block likely critical/system PIDs on common platforms
        String os = System.getProperty("os.name", "").toLowerCase();
        if (os.contains("win")) {
            if (pid <= 4) {
                return false;
            }
        } else {
            if (pid <= 1) {
                return false;
            }
        }

        try {
            // If process doesn't exist, it's already "terminated"
            var opt = ProcessHandle.of(pid);
            if (opt.isEmpty()) {
                return true;
            }
            ProcessHandle handle = opt.get();

            // Try graceful termination
            if (handle.isAlive()) {
                handle.destroy();
            }

            if (handle.isAlive()) {
                CompletableFuture<ProcessHandle> cf = handle.onExit();
                try {
                    cf.get(2, TimeUnit.SECONDS);
                } catch (Exception ignored) { }
            }

            // If still alive, force terminate
            opt = ProcessHandle.of(pid);
            if (opt.isPresent() && opt.get().isAlive()) {
                opt.get().destroyForcibly();
                CompletableFuture<ProcessHandle> cf2 = opt.get().onExit();
                try {
                    cf2.get(3, TimeUnit.SECONDS);
                } catch (Exception ignored) { }
            }

            opt = ProcessHandle.of(pid);
            return opt.isEmpty() || !opt.get().isAlive();
        } catch (SecurityException se) {
            return false;
        } catch (Exception e) {
            return false;
        }
    }

    // Helper to parse long safely
    private static Long parseLongSafe(String s) {
        try {
            return Long.parseLong(s.trim());
        } catch (Exception e) {
            return null;
        }
    }

    public static void main(String[] args) {
        List<Long> pids = new ArrayList<>();
        if (args != null && args.length > 0) {
            for (String a : args) {
                Long v = parseLongSafe(a);
                if (v != null) pids.add(v);
            }
        } else {
            long self = ProcessHandle.current().pid();
            // Five test cases (safe/demonstrative)
            pids.add(-1L);       // invalid
            pids.add(0L);        // invalid
            pids.add(1L);        // likely critical on POSIX
            pids.add(self);      // self (blocked)
            pids.add(999999L);   // likely non-existent
            System.out.println("No arguments provided. Running 5 demonstrative test cases.");
        }

        for (long pid : pids) {
            boolean result = terminateProcess(pid);
            System.out.println("Terminate PID " + pid + ": " + (result ? "SUCCESS/NOT ALIVE" : "FAILED/ALIVE"));
        }
    }
}