import java.util.*;
import java.io.*;

public class Task75 {

    public static boolean terminateProcess(long pid) {
        Optional<ProcessHandle> opt = ProcessHandle.of(pid);
        if (!opt.isPresent()) return false;
        ProcessHandle ph = opt.get();
        if (!ph.isAlive()) return true;

        ph.destroy();
        for (int i = 0; i < 20; i++) {
            if (!ph.isAlive()) return true;
            try { Thread.sleep(100); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        }

        if (ph.isAlive()) ph.destroyForcibly();
        for (int i = 0; i < 20; i++) {
            if (!ph.isAlive()) return true;
            try { Thread.sleep(100); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        }
        return !ph.isAlive();
    }

    private static boolean isWindows() {
        String os = System.getProperty("os.name").toLowerCase(Locale.ROOT);
        return os.contains("win");
    }

    private static Process spawnDummyProcess() throws IOException {
        if (isWindows()) {
            return new ProcessBuilder("cmd.exe", "/c", "ping -n 30 127.0.0.1 > nul").redirectErrorStream(true).start();
        } else {
            return new ProcessBuilder("sh", "-c", "sleep 30").redirectErrorStream(true).start();
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length > 0) {
            for (String s : args) {
                try {
                    long pid = Long.parseLong(s);
                    boolean ok = terminateProcess(pid);
                    System.out.println("PID " + pid + " terminated: " + ok);
                } catch (NumberFormatException nfe) {
                    System.out.println("Invalid PID: " + s);
                }
            }
            return;
        }

        // 5 test cases: spawn 5 dummy processes and terminate them
        List<Process> procs = new ArrayList<>();
        List<Long> pids = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            Process p = spawnDummyProcess();
            procs.add(p);
            long pid = p.pid();
            pids.add(pid);
            System.out.println("Spawned dummy process PID: " + pid);
        }
        for (long pid : pids) {
            boolean ok = terminateProcess(pid);
            System.out.println("Test terminate PID " + pid + ": " + ok);
        }
    }
}