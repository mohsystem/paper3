import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse114 {

    /**
     * Creates a command string from a format and arguments, then executes it.
     * The system function (ProcessBuilder) takes this string and executes it
     * via a shell to interpret it correctly.
     * @param format The format string for the command (e.g., "ls %s").
     * @param args The arguments to be formatted into the string.
     */
    public static void executeCommand(String format, String... args) {
        // The "print function" (String.format) is used to create the string
        // that is passed to the system function.
        String command = String.format(format, (Object[]) args);
        
        System.out.println("--- Executing command: " + command + " ---");
        try {
            // Using a shell to execute the command string is similar to C's system()
            String[] shellCommand = {"/bin/sh", "-c", command};
            ProcessBuilder pb = new ProcessBuilder(shellCommand);
            
            // Merge standard output and standard error for simpler reading
            pb.redirectErrorStream(true);
            Process process = pb.start();

            // Read the output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            // Wait for the process to complete and get the exit code
            int exitCode = process.waitFor();
            System.out.println("Exited with code: " + exitCode);
            System.out.println("-----------------------------------------\n");

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: '" + command + "'");
            e.printStackTrace();
        }
    }

    /**
     * Main method with 5 test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Test Case 1: simple ls
        executeCommand("ls");

        // Test Case 2: ls with -l flag
        executeCommand("ls %s", "-l");

        // Test Case 3: ls with -a flag
        executeCommand("ls %s", "-a");

        // Test Case 4: ls a specific existing path
        executeCommand("ls %s", ".");

        // Test Case 5: ls a non-existent path (should produce an error)
        executeCommand("ls %s", "/non_existent_directory_xyz123");
    }
}