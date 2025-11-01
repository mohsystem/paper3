import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Lse114 {

    /**
     * Executes the 'ls' command with specified arguments in a secure way.
     * This method uses ProcessBuilder, which is the recommended secure way to
     * execute external commands in Java. It treats each argument as a distinct
     * string, preventing shell injection vulnerabilities that can occur when
     * concatenating a single command string from user input.
     *
     * @param args A variable number of string arguments for the 'ls' command (e.g., "-l", "/tmp").
     */
    public static void executeLs(String... args) {
        // The command and its arguments are passed as a list of strings.
        // This is crucial for security as it avoids shell interpretation.
        List<String> command = new ArrayList<>();
        command.add("ls");
        Collections.addAll(command, args);

        System.out.println("Executing: " + String.join(" ", command));
        System.out.println("------------------------------------");

        ProcessBuilder pb = new ProcessBuilder(command);
        // Redirecting error stream to the standard output stream for easier reading.
        pb.redirectErrorStream(true);

        try {
            Process process = pb.start();

            // Read the output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("------------------------------------");
            System.out.println("Exited with code: " + exitCode);
            System.out.println();

        } catch (IOException | InterruptedException e) {
            System.err.println("Failed to execute command.");
            // In a real application, more specific error handling would be needed.
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: ls -l . ---");
        executeLs("-l", ".");

        System.out.println("--- Test Case 2: ls -a / ---");
        // Using a common, safe directory path.
        executeLs("-a", "/");

        System.out.println("--- Test Case 3: ls (no arguments) ---");
        executeLs();

        System.out.println("--- Test Case 4: ls /tmp ---");
        executeLs("/tmp");

        System.out.println("--- Test Case 5: ls /non_existent_directory_xyz ---");
        executeLs("/non_existent_directory_xyz");
    }
}