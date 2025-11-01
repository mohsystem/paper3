import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.stream.Collectors;

public class Lse114 {

    /**
     * Executes the 'ls' command with specified arguments by building a command string.
     * WARNING: This method constructs a command string to be executed by a shell.
     * Passing unsanitized user input to this function is extremely dangerous and can
     * lead to command injection vulnerabilities. This code is for demonstrating the
     * described pattern only and should NOT be used in production with untrusted input.
     *
     * @param args A variable number of string arguments for the 'ls' command (e.g., "-l", "/tmp").
     */
    public static void executeLsCommand(String... args) {
        // Use a "print" style function (String.format) to create the command string.
        String arguments = Arrays.stream(args)
                                 .filter(s -> s != null && !s.isEmpty())
                                 .collect(Collectors.joining(" "));
        String command = String.format("ls %s", arguments).trim();

        System.out.println("--- Executing command: '" + command + "' ---");

        try {
            // Using "sh -c" to explicitly invoke a shell, which mimics system().
            Process process = new ProcessBuilder("sh", "-c", command).start();

            // Read standard output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            // Read standard error from the command
            try (BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                String line;
                while ((line = errorReader.readLine()) != null) {
                    System.err.println(line);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("--- Command finished with exit code: " + exitCode + " ---\n");

        } catch (IOException | InterruptedException e) {
            System.err.println("Failed to execute command: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Test Case 1: ls
        System.out.println(">>> Test Case 1: ls (no arguments)");
        executeLsCommand();

        // Test Case 2: ls -l
        System.out.println(">>> Test Case 2: ls -l");
        executeLsCommand("-l");

        // Test Case 3: ls -a -h
        System.out.println(">>> Test Case 3: ls -a -h");
        executeLsCommand("-a", "-h");

        // Test Case 4: ls on a specific (likely existing) directory, e.g., /tmp
        // Using a path that is likely to exist on Unix-like systems.
        System.out.println(">>> Test Case 4: ls -l /tmp");
        executeLsCommand("-l", "/tmp");

        // Test Case 5: ls on a non-existent directory to show error handling
        System.out.println(">>> Test Case 5: ls /non_existent_dir_12345");
        executeLsCommand("/non_existent_dir_12345");
    }
}