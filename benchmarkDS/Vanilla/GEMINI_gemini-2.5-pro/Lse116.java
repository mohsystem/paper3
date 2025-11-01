import java.io.IOException;

public class Lse116 {

    /**
     * Executes a system command using a shell.
     * @param command The command string to execute.
     * @return The exit code of the command. Returns -1 on error.
     */
    public static int executeCommand(String command) {
        try {
            ProcessBuilder pb = new ProcessBuilder();
            // This setup emulates the C system() call, which uses a shell.
            // On Unix-like systems, this is typically "/bin/sh -c".
            pb.command("sh", "-c", command);
            // Redirects the subprocess's standard output and error to the current process's console.
            pb.inheritIO();
            Process process = pb.start();
            // Wait for the command to complete and get the exit code.
            int exitCode = process.waitFor();
            return exitCode;
        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            return -1;
        }
    }

    public static void main(String[] args) {
        String[] testCommands = {
            "ls",
            "ls -l",
            "ls -a",
            "ls -al",
            "ls non_existent_directory"
        };

        for (int i = 0; i < testCommands.length; i++) {
            // Using String.format to build the output string, as mentioned in the prompt.
            String header = String.format("--- Test Case %d: Executing '%s' ---", i + 1, testCommands[i]);
            System.out.println(header);
            
            int exitCode = executeCommand(testCommands[i]);

            String footer = String.format("--- Command exited with code: %d ---\n", exitCode);
            System.out.println(footer);
        }
    }
}