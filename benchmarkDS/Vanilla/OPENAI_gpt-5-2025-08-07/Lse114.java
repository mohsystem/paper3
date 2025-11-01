import java.util.*;
import java.io.*;

public class Lse114 {
    // "print" function: creates a formatted string from a format and variable arguments.
    public static String print(String format, Object... args) {
        return String.format(format, args);
    }

    // "system" function equivalent: executes a shell command string.
    public static int systemExec(String command) {
        try {
            Process p = new ProcessBuilder("/bin/sh", "-c", command)
                    .inheritIO()
                    .start();
            return p.waitFor();
        } catch (Exception e) {
            return -1;
        }
    }

    // Convenience function: format and execute in one step.
    public static int systemExecf(String format, Object... args) {
        String cmd = print(format, args);
        return systemExec(cmd);
    }

    // Example test cases
    public static void main(String[] args) {
        // 5 test cases using ls with different arguments
        systemExecf("ls");
        systemExecf("ls %s", "-l");
        systemExecf("ls %s %s", "-l", "-a");
        systemExecf("ls %s %s %s", "-l", "-a", ".");
        systemExecf("ls %s", ".");
    }
}