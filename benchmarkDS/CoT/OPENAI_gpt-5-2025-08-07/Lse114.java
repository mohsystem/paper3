import java.io.IOException;

public class Lse114 {

    // "print" function: creates a formatted string from a format and variable arguments
    public static String print(String fmt, Object... args) {
        if (args == null || args.length == 0) return fmt;
        return String.format(fmt, args);
        }

    // "system" function: executes a command via the system shell and returns the exit code
    public static int system(String command) throws IOException, InterruptedException {
        Process p = new ProcessBuilder("sh", "-c", command).inheritIO().start();
        return p.waitFor();
    }

    // Convenience function combining print + system
    public static int execf(String fmt, Object... args) throws IOException, InterruptedException {
        String cmd = print(fmt, args);
        return system(cmd);
    }

    // 5 test cases
    public static void main(String[] args) throws Exception {
        int r1 = execf("ls");
        int r2 = execf("ls %s", "-l");
        int r3 = execf("ls %s %s", "-a", ".");
        int r4 = execf("ls %s", "--color=auto");
        int r5 = execf("ls %s", "..");

        System.out.println("Exit codes:");
        System.out.println(r1);
        System.out.println(r2);
        System.out.println(r3);
        System.out.println(r4);
        System.out.println(r5);
    }
}