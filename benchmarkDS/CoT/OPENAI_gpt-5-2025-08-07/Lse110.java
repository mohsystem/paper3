// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement a function that takes a username, runs `/usr/bin/id -- <username>`, and returns exit status. Print the exit status.
// 2) Security requirements: Avoid shell, use absolute path, prevent option injection by using "--", inherit IO safely, handle exceptions.
// 3) Secure coding generation: Use ProcessBuilder with argument list, inherit IO, wait for process completion, handle null/empty input, and exceptions.
// 4) Code review: Checked for resource leaks (inheritIO avoids stream handling), shell injection avoided, "--" prevents option parsing of username, absolute path used.
// 5) Secure code output: Final code with robust error handling, non-zero exit on failure, and main with 5 test cases.

import java.util.Arrays;

public class Lse110 {
    public static int runId(String username) {
        if (username == null || username.isEmpty()) {
            return 2; // invalid input
        }
        ProcessBuilder pb = new ProcessBuilder("/usr/bin/id", "--", username);
        pb.inheritIO(); // let the child print directly to stdout/stderr
        try {
            Process p = pb.start();
            int exitCode = p.waitFor();
            return exitCode;
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
            return 130; // interrupted
        } catch (Exception ex) {
            return 127; // command not found or other failure
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length >= 1) {
            int status = runId(args[0]);
            System.out.println("Exit status: " + status);
            return;
        }
        // 5 test cases
        String[] tests = new String[] {
            "root",
            "nobody",
            "daemon",
            "www-data",
            "user_does_not_exist_123456"
        };
        for (String u : tests) {
            int status = runId(u);
            System.out.println("Username: " + u + " -> Exit status: " + status);
        }
    }
}